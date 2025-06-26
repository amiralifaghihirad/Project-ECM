// Create a new WebSocket connection to the server
const ws = new WebSocket('ws://localhost:8080');

// Event listener for receiving messages from the server
ws.onmessage = (event) => {
  // Parse the received JSON data
  const data = JSON.parse(event.data);

  // Call the function to update the chart with new data
  updateChart(data);
};

// Get the 2D drawing context of the canvas element with id 'myChart'
const ctx = document.getElementById('myChart').getContext('2d');

// Create a new line chart using Chart.js
const myChart = new Chart(ctx, {
  type: 'line', // Chart type: line
  data: {
    labels: [], // Initialize empty labels array
