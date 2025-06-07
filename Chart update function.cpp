function updateChart(data) {
  myChart.data.labels.push(new Date().toLocaleTimeString());
  myChart.data.datasets[0].data.push(data.value);
  
  if (myChart.data.labels.length > 20) {
    myChart.data.labels.shift();
    myChart.data.datasets[0].data.shift();
  }

  myChart.update();
}