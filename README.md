<p align="center">
    <img src="orbytrack_name_logo.png" style="width:auto;height:auto;max-height:120px;" />
</p>
<p align="center">
<img src="Design\Icons\YT_Thumbnail.png" style="width:auto;height:auto;max-height:250px;" />
<img src="Design\Photos\_JSN9351.jpg" style="width:auto;height:auto;max-height:250px;" />
</p>

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)  
[![3D Printable](https://img.shields.io/badge/3D%20Printable-Yes-blue.svg)](https://github.com/yourusername/OrbyTrack/tree/main/Case)  

**OrbyTrack** is a portable, battery-operated device designed to measure the flow rate of an IV drip. Built using the Adafruit Feather M0 module, Adafruit OLED (128x64), and IR-based emitter-receiver drop detection, it provides healthcare professionals with a compact and efficient solution. For nurses working in high-pressure environments such as intensive care units or emergency rooms, OrbyTrack ensures accurate, real-time monitoring of IV flow rates. This accuracy is vital in life-critical moments, helping to prevent over-infusion or under-infusion of fluids, which could have severe consequences for patient outcomes. By providing clear and precise data, OrbyTrack empowers nurses to make informed decisions swiftly and confidently, ensuring patient safety remains the top priority. 



## Features  

- **Accurate Flow Measurement:** IR-based technology for precise IV drip monitoring.  
- **Portable and Battery-Operated:** With 500mAh battery, it can provide 20 hours of continuous usage.  
- **User-Friendly Interface:** Simple control with three tactile buttons and visible screen.  
- **Customizable:** Fully 3D-printable casing.  



## Project Structure  

```markdown
OrbyTrack/  
│  
├── Firmware/     # Codebase for Adafruit Feather M0  
├── Design/       # 3D design files for the casing  
├── Assembly/     # Step-by-step assembly instructions  
└── Usage/        # User guide, calibration, and troubleshooting 
```


## Hardware Overview  

| **Component**       | **Details**                 |  
|----------------------|-----------------------------|  
| Microcontroller      | Atmel SAMD21       |  
| Module              | Adafruit M0    | 
| Display              | Adafruit OLED (128x64)     |  
| Buttons              | 3 tactile buttons          |  
| Sensors              | IR emitter-receiver  |  
| Power Source         | Battery-operated (500mAh)        |  



## License  

This project is licensed under the MIT License. See the `LICENSE` file for details. 