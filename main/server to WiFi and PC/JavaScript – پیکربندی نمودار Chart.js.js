    datasets: [{
      // Label for the dataset shown in the chart legend
      label: 'مقدار حسگر', // Sensor Value (if you'd like, can translate this too)

      // Array to store the data points
      data: [],

      // Color of the line
      borderColor: 'blue',

      // Disable filling the area under the line
      fill: false
    }]
  },
  options: {
    // Make the chart responsive to window size changes
    responsive: true
  }
});
