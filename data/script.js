const movingAverage = (data, windowSize) => {
    const averagedData = new Float32Array(data.length);
    for (let i = 0; i < data.length; i++) {
        let sum = 0;
        let count = 0;
        for (let j = Math.max(0, i - windowSize); j <= i; j++) {
            sum += data[j];
            count++;
        }
        averagedData[i] = sum / count;
    }
    return averagedData;
};

// ใช้ Moving Average กับ channelData ก่อนเล่นเสียง
const filteredData = movingAverage(channelData, 3); // Window size = 3
for (let i = 0; i < audioData.length; i++) {
    channelData[i] = filteredData[i];
}
