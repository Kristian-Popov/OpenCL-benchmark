import Tabulator from "tabulator-tables";

const durationDoubleNsId: string = 'durationDoubleNs';

interface TableValue {
    toTableString(): string;
}

interface Unit {
    multiplier: number;
    description: string;
}

interface Numberlike {
    asNumber(): number;
}

class UnitPicker {
    units: Array<Unit>;
    constructor(units: Array<Unit>) {
        // Clone an array
        this.units = [...units];
        // Sort by multiplier in descending order
        this.units.sort((n1: any, n2: any) => n2.multiplier - n1.multiplier);
    }

    select(d: Numberlike): Unit {
        // Find the biggest unit that is smaller than input value
        let selectedUnit;
        for(let unit of this.units) {
            if((<any>unit).multiplier < d.asNumber()) {
                selectedUnit = unit;
                break;
            }
        }
        if(selectedUnit == null) {
            selectedUnit = this.units[this.units.length - 1]; // Use the smallest available unit
        }
        return selectedUnit;
    }
}
const durationUnits: Array<Unit> = [
    {multiplier: 1.0e-9, description: 'ns'},
    {multiplier: 1.0e-6, description: 'mcs'},
    {multiplier: 1.0e-3, description: 'ms'},
    {multiplier: 1, description: 's'},
    {multiplier: 60, description: 'min'}
];
const durationUnitPicker: UnitPicker = new UnitPicker(durationUnits);

const throughtputUnits: Array<Unit> = [
    {multiplier: 1, description: ''},
    {multiplier: 1000, description: 'K'},
    {multiplier: 1.0e6, description: 'M'},
    {multiplier: 1.0e9, description: 'G'},
];
const throughtputUnitPicker: UnitPicker = new UnitPicker(throughtputUnits);

class Duration implements TableValue, Numberlike {
    durationDoubleNs: number;

    static fromSerializedJson(data: object): Duration
    {
        return new this(data[durationDoubleNsId]);
    }

    constructor(nanoseconds: number = 0) {
        this.durationDoubleNs = nanoseconds;
    }

    toTableString(): string {
        let unit: Unit = durationUnitPicker.select(this);
        return this.calcValueWithMultiplier(unit.multiplier).toFixed(2) + ' ' + unit.description;
    }

    asSeconds(): number {
        return this.calcValueWithMultiplier();
    }

    asNanoseconds(): number {
        return this.durationDoubleNs;
    }

    asNumber(): number {
        return this.asSeconds();
    }

    divide(divisor: number): Duration {
        return new Duration(this.asNanoseconds() / divisor);
    }

    private calcValueWithMultiplier(multiplier: number = 1.0): number {
        return this.durationDoubleNs * (1.0e-9 / multiplier);
    }
}

const avgDurationId: string = "avg";
class DurationRange implements TableValue {
    avgDuration: Duration;

    // Calculates duration for a given step
    constructor(fixtureData: object, stepId: string) {
        if(fixtureData["compressedDuration"]) {
            this.avgDuration = Duration.fromSerializedJson(fixtureData["compressedDuration"][stepId][avgDurationId]);
        } else if(fixtureData["fullDuration"]) {
            // TODO calculate average values here
            let durationList: object[] = fixtureData["fullDuration"][stepId];
            if( durationList.length !== 1 ) {
                throw "Multiple entries in fullDuration list is not supported yet!";
            }
            this.avgDuration = Duration.fromSerializedJson(durationList[0]);
        }
    }

    toTableString(): string {
        return this.avgDuration.toTableString();
    }

    asSeconds(): number {
        return this.avgDuration.asSeconds();
    }

    asNanoseconds(): number {
        return this.avgDuration.asNanoseconds();
    }
}

class ElementProcessingTimeIndicator implements TableValue {
    avgDuration: Duration;
    elementCount: number;
    static createStepIndicator(fixtureData: object, elementCount: number, stepId: string): ElementProcessingTimeIndicator {
        let durationRange = new DurationRange(fixtureData, stepId);
        return new ElementProcessingTimeIndicator(new Duration(durationRange.asNanoseconds() / elementCount));
    }

    static createTotalIndicator(fixtureData: object, elementCount: number): ElementProcessingTimeIndicator {
        return new ElementProcessingTimeIndicator(calculateTotalDuration(fixtureData).divide(elementCount));
    }

    constructor(avgDuration: Duration) {
        this.avgDuration = avgDuration;
    }

    toTableString(): string {
        return this.avgDuration.toTableString();
    }
}

class ThroughputIndicator implements TableValue, Numberlike {
    throughput: number;
    constructor(fixtureData: object, elementCount: number) {
        this.throughput = elementCount / calculateTotalDuration(fixtureData).asSeconds();
    }

    toTableString(): string {
        let unit: Unit = throughtputUnitPicker.select(this);
        return (this.throughput / unit.multiplier).toFixed(2) + unit.description;
    }

    asNumber(): number {
        return this.throughput;
    }
}

const elementCountId: string = "elementCount";
const durationIndicatorId = 'DurationIndicator';
const elementProcessingTimeIndicatorId = 'ElementProcessingTimeIndicator';
const throughtputIndicatorId = 'ThroughputIndicator';
const failureIndicatorId = 'FailureIndicator';
const totalDurationStepId = 'totalDuration';
const totalDurationStepTitle = "Total duration"
const indicators: Object = { // TODO simplify this thing or remove altogether
    [durationIndicatorId]: {
        name: 'Duration indicator',
    },
    [elementProcessingTimeIndicatorId]: {
        name: 'Element processing time indicator',
    },
    [throughtputIndicatorId]: {
        name: 'Throughtput',
    },
};

function getDeviceName(fixtureId: string): string {
    // TODO stub for now, separate device name and algorithm
    return fixtureId;
}

function getPlatformName(platformList: object, deviceName: string): string {
    for(let platformName in platformList) {
        if(platformList[platformName].indexOf(deviceName) >= 0) {
            return platformName;
        }
    }
}

// Alternative for Object.values
// Based on https://stackoverflow.com/a/42966443
function getObjectValues(o: object): any[] {
    return Object.keys(o).map(key => o[key]);
}

function createStepSubcolumns(indicatorId: string, steps: string[]): object {
    let subcolumns = getObjectValues(steps).map(stepId => { // Add subcolumn for every step
        let fullIndicatorId = `${indicatorId}-${stepId}`;
        return {title: stepId, field:fullIndicatorId};
    } );
    let fullIndicatorId = `${indicatorId}-${totalDurationStepId}`; // Add subcolumn for total ducation
    subcolumns.push({title: totalDurationStepTitle, field:fullIndicatorId});

    let indicatorTitle: string = indicators[indicatorId].name || indicatorId;
    return {title: indicatorTitle, columns: subcolumns};
}

function calculateTotalDuration(fixtureData: object): Duration {
    let ns = 0;
    let data = fixtureData["compressedDuration"] || fixtureData["fullDuration"];
    for(let stepId in data) {
        ns += new DurationRange(fixtureData, stepId).asNanoseconds();
    }
    return new Duration(ns);
}

function processFixtureDuration(fixtureData: object, elementCount: number | null, val: object)
{
    let durations: object = fixtureData["compressedDuration"] || fixtureData["fullDuration"];

    // Create values for duration indicator
    for(let stepId in durations) {
        let fullIndicatorId = `${durationIndicatorId}-${stepId}`;
        let indicator: TableValue = new DurationRange(fixtureData, stepId);
        val[fullIndicatorId] = indicator.toTableString();
    }
    val[`${durationIndicatorId}-${totalDurationStepId}`] = calculateTotalDuration(fixtureData).toTableString();

    if( typeof elementCount === "number" ) {
        // Create values for element processing time indicator
        for(let stepId in durations) {
            let fullIndicatorId = `${elementProcessingTimeIndicatorId}-${stepId}`;
            let indicator: TableValue = ElementProcessingTimeIndicator.createStepIndicator(fixtureData, elementCount, stepId);
            val[fullIndicatorId] = indicator.toTableString();
        }
        // TODO take steps into account
        val[`${elementProcessingTimeIndicatorId}-${totalDurationStepId}`] =
            ElementProcessingTimeIndicator.createTotalIndicator(fixtureData, elementCount).toTableString();

        // Create values for throughput indicator
        let indicator: TableValue = new ThroughputIndicator(fixtureData, elementCount);
        val[throughtputIndicatorId] = indicator.toTableString();
    }
}

function createColumns(fixtureFamilyData: object): object[] {
    let steps: string[] = fixtureFamilyData["steps"];

    let columnMap: object = {
        [durationIndicatorId]: createStepSubcolumns(durationIndicatorId, steps)
    };

    if( typeof fixtureFamilyData["elementCount"] === "number" ) {
        // Create columns for a element processing time indicator
        columnMap[elementProcessingTimeIndicatorId] = createStepSubcolumns(elementProcessingTimeIndicatorId, steps);

        // Create column for throughput indicator
        columnMap[throughtputIndicatorId] = {title: indicators[throughtputIndicatorId].name, field: throughtputIndicatorId};
    }

    /*
    let atLeastOneFixtureFailed: boolean = fixtureFamilyData["fixtures"].some(function(fixtureData: object) {
        return fixtureData["failureReason"] != null;
    });
    */

    return [
        {field:"platform", visible: false, sorter:"string"},
        {title:"Device/Algorithm", field:"fixtureId", sorter:"string"},
        {title:"Iteration count", field:"iterationCount"},
        // TODO merge cells together in a row to show this would be helpful (probably requires col span not supported yet)
        // It would be cool to show failure reason column only if it is needed, but when it is hidden it ates away all previous ones
        //{title:"Failure reason", visible: atLeastOneFixtureFailed, field:failureIndicatorId},
        {title:"Failure reason", visible: true, field:failureIndicatorId},
        columnMap[durationIndicatorId],
        columnMap[elementProcessingTimeIndicatorId],
        columnMap[throughtputIndicatorId]
    ];
}

function updateBenchmarkView(benchmarkData: any) {
    // Clear benchmark data
    document.getElementById('benchmarkContent').innerHTML = "";

    // Create new data
    let content = document.createElement("div");
    document.getElementById('benchmarkContent').appendChild(content);

    let time = new Date(benchmarkData.baseInfo.time);
    let timeText = document.createElement("p");
    timeText.innerHTML = 'Benchmark data were generated at ' + time.toString();
    content.appendChild(timeText);

    let platformList: Object = benchmarkData.deviceList;
    let deviceList: string[] = [];
    for(let platformName in platformList) {
        deviceList = deviceList.concat(platformList[platformName]);
    }

    let benchmarkResults = benchmarkData["fixtureFamilies"];
    for (let fixtureFamilyData of benchmarkResults) {
        let fixtureFamilyNameText = document.createElement("p");
        fixtureFamilyNameText.innerHTML = fixtureFamilyData.name;
        content.appendChild(fixtureFamilyNameText);

        let rowId: number = 0;
        let data: Object[] = [];

        for (let fixtureData of fixtureFamilyData["fixtures"]) {
            let fixtureId = fixtureData.name;
            let val = {
                id: rowId,
                fixtureId: fixtureId,
                platform: getPlatformName(platformList, getDeviceName(fixtureId)),
                iterationCount: fixtureData.iterationCount
            };

            if(fixtureData["failureReason"]) {
                //columnMap[failureIndicatorId] = {title: indicators[failureIndicatorId].name, field: failureIndicatorId};
                val[failureIndicatorId] = fixtureData["failureReason"];
            } else {
                processFixtureDuration(fixtureData, fixtureFamilyData[elementCountId], val);
            }

            data.push(val);
            rowId++;
        }

        let tableBase = document.createElement("div");
        let table = new Tabulator(tableBase, {
            layout:"fitDataFill",
            data: data,
            columns: createColumns(fixtureFamilyData),
            groupBy:"platform",
        });
        content.appendChild(tableBase);
    }
}

function onFilesUpdated(files: FileList) {
    console.log('Starting to load file');
    const selectedFile = (<HTMLInputElement>document.getElementById('fileInput')).files[0];
    let reader = new FileReader();
    reader.addEventListener("loadend", function () {
        console.log('File loaded, started processing');
        updateBenchmarkView(JSON.parse(<string>reader.result));
        console.log('Processing finished');
    });
    reader.readAsText(selectedFile);
}

window.onload = () => {
    document.getElementById('fileInput').onchange = () => onFilesUpdated(this.files);
};