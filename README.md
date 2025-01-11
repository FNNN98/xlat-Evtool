# xlat-Evtool

<h3 align="left"> Introduction:</h3>
  
The XLAT-Evtool is a complementary application to the XLAT designed for developers, reviewers, and enthusiasts.

It makes use of the J-Link Virtual COM Port to read and parse data in real time directly from the XLAT. It extrapolates metrics such as lowest/maximum latency but also Median Absolute Deviation (MAD), Interquartile Range (IQR), percentiles, ... thus extending the functionalities of the XLAT.

Similar to the LDAT, it also makes it possible to export and import XLAT results to a CSV file. The CSV file not only includes the parsed data, but also features a list of the advanced metrics described earlier.

Last but not least, it makes use of Qt Graphics View Framework to provide a 16 steps latency distribution chart dynamically calculated based on data distribution and a Scatter Plot to visually assess latency behaviour over time.


   <h3 align="left"> Main Interface:</h3>    
<img src="https://github.com/FNNN98/xlat-Evtool/blob/main/pic2.png?raw=true" width="650"> 
   <h3 align="left"> Data Visualization Tools:</h3>
<div style="display:flex;">
    <div style="display:flex; flex-direction:column;">
        <img src="https://github.com/FNNN98/xlat-Evtool/blob/main/pic3.png?raw=true" width="500">
        <img src="https://github.com/FNNN98/xlat-Evtool/blob/main/pic1.png?raw=true" width="500">
    </div>
</div>

<h2 align="left"> Setup procedure:</h2>

- Install J-Link drivers
- Enable J-Link VCOM:
  
  https://wiki.segger.com/Using_J-Link_VCOM_functionality

  _J-Link Configurator method is recommended_
- Download and extract the compressed file in a new folder
- Click on the .exe file to launch the program

<h3 align="left">Languages and Tools:</h3>
<p align="left"> <a href="https://www.w3schools.com/cpp/" target="_blank" rel="noreferrer"> <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/cplusplus/cplusplus-original.svg" alt="cplusplus" width="40" height="40"/> </a> <a href="https://www.qt.io/" target="_blank" rel="noreferrer"> <img src="https://upload.wikimedia.org/wikipedia/commons/0/0b/Qt_logo_2016.svg" alt="qt" width="40" height="40"/> </a> </p>

Informations and dependencies

    Compiled and tested with Desktop 5.12.11 MinGW 32-bit and Desktop 5.12.11 MinGW 64-bit.
    Qt Framework
    C++ Standard Library


<h4 align="left"> Disclaimer:</h4>

XLAT is a registered trademark of Finalmouse. The use of the xlat-Evtool name does not indicate endorsement by or affiliation with Finalmouse , and is used here under fair use principles as an open source external evaluation tool released under GPLv3 aimed at developers, enthusiasts and reviewers.


