// Function to update the chart with new incoming data
function updateChart(data) {
  // Add the current time as a new label on the x-axis
  myChart.data.labels.push(new Date().toLocaleTimeString());

  // Add the new sensor value to the dataset
  myChart.data.datasets[0].data.push(data.value);
  
  // Keep only the latest 20 data points by removing the oldest one if needed
  if (myChart.data.labels.length > 20) {
    myChart.data.labels.shift(); // Remove the first label
    myChart.data.datasets[0].data.shift(); // Remove the first data point
  }

  // Update the chart to reflect the new data
  myChart.update();
}
