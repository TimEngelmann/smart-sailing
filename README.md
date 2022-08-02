# smart-sailing
So I love sailing and this time I figured why not collect some data along the way. Especially, because there was a new sail (a Code0) on board and we wanted to see how well it does. So here is how this project is structured:
1. Record the data the internal boat system provides -> sail_data
2. Analysis of this data -> sail_analysis
3. Bonus: Try measuring the pressure in the sail (too noisy) -> sail_pressure

## sail_data
The boat has an internal system, that records all kinds of data already. Wind Speed, Wind Angle, Boat Speed, and lots more. But unfortunately, it's not too easy to record and save that data. So we thought of a workaround. There is an App called Raymarine Control. It lets you stream and control the plotter from an iPhone/iPad/Mac. And the plotter can display all the data we are interested in on a single page. So we streamed that view on a Mac and wrote a python program to extract the relevant numbers. Every second it records the current wind/boat data and writes them to a CSV. On startup, it asks what sails are up at the moment (Code0, Genua, or Reffed to a certain level). Here is what data we recorded:
- time: Timestamp
- reffen: [-1: Code0 was up, 0: Genua, 1: Reefed to first dot, 2: Reefed to second dot, 3: Reefed to third dot]
- awa: Apparent Wind Angle
- aws: Apparent Wind Speed
- sog: Speed over Ground
- rollen: Tilt of boat in longitude
- twa: True Wind Angle
- tws: True Wind Speed
- stw: Speed through Water
- kurs: Heading of boat

## sail_analysis
Next, it was time to generate some insights. Check out the jupyter notebook for more details. In short, the bigger foresail (Code0) does make the boat go faster across various wind speeds. And our crew did not quite reach the performance that a professional crew did.

## sail_pressure
In addition to the boat data, we somehow wanted to include the trim of the sail in our models. So our idea was to measure the pressure needed to turn the winch further. However, we quickly realized that the recorded data was way too noisy and was barely correlated with the current trim of the sail. However, for the sake of completeness, here the circuit diagram of our "measuring tool":
<img src="https://user-images.githubusercontent.com/46136690/182480501-4e4adc49-e393-4f2a-9d39-cc150c05f4f1.jpg" width="400">
The microcontroller connected to the hotspot of my phone and wrote its pressure measurements in batches to a firebase database. This data could then be matched with the other data via the timestamp.
