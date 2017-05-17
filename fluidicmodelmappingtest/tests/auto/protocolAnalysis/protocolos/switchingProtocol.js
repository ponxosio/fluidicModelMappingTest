{
  "tittle": "switchingFlows",
  "linkedBlocks": [
    [
      {
        "timeOfOperation": "0",
        "timeOfOperation_units": "s",
        "linked": "FALSE",
        "duration": "30",
        "duration_units": "s",
        "block_type": "continuous_flow",
        "continuosflow_type": "1",
        "source": {
          "block_type": "container",
          "containerName": "A",
          "type": "1",
          "destiny": "Ambient",
          "initialVolume": "0",
          "initialVolumeUnits": "ml"
        },
        "destination": {
          "block_type": "container",
          "containerName": "B",
          "type": "1",
          "destiny": "Ambient",
          "rate": {
            "block_type": "math_number",
            "value": "300"
          },
          "rate_volume_units": "ml",
          "rate_time_units": "hr",
          "initialVolume": "0",
          "initialVolumeUnits": "ml"
        }
      }
    ],
    [
      {
        "timeOfOperation": "30",
        "timeOfOperation_units": "s",
        "linked": "FALSE",
        "duration": "30",
        "duration_units": "s",
        "block_type": "continuous_flow",
        "continuosflow_type": "1",
        "source": {
          "block_type": "container",
          "containerName": "D",
          "type": "1",
          "destiny": "Ambient",
          "initialVolume": "0",
          "initialVolumeUnits": "ml"
        },
        "destination": {
          "block_type": "container",
          "containerName": "B",
          "type": "1",
          "destiny": "Ambient",
          "rate": {
            "block_type": "math_number",
            "value": "300"
          },
          "rate_volume_units": "ml",
          "rate_time_units": "hr",
          "initialVolume": "0",
          "initialVolumeUnits": "ml"
        }
      }
    ],
    [
      {
        "timeOfOperation": "0",
        "timeOfOperation_units": "s",
        "linked": "FALSE",
        "duration": "60",
        "duration_units": "s",
        "block_type": "continuous_flow",
        "continuosflow_type": "1",
        "source": {
          "block_type": "container",
          "containerName": "B",
          "type": "1",
          "destiny": "Ambient",
          "initialVolume": "0",
          "initialVolumeUnits": "ml"
        },
        "destination": {
          "block_type": "container",
          "containerName": "C",
          "type": "1",
          "destiny": "Ambient",
          "rate": {
            "block_type": "math_number",
            "value": "300"
          },
          "rate_volume_units": "ml",
          "rate_time_units": "hr",
          "initialVolume": "0",
          "initialVolumeUnits": "ml"
        }
      }
    ]
  ]
}