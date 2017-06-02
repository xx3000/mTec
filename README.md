mTec - Efficient Rendering with Distance Fields
========================================
Master Thesis by Michael Mroz

![](Figures/3.PNG)

In modern real-time rendering, there is a large focus on producing photorealistic qualities in images. Most techniques used to produce these effects were developed with older, less powerful hardware in mind and sacrifice visual fidelity for fast computation. Now that more and more powerful hardware has been developed, research into alternatives for these well-established techniques has gained traction in the field of computer graphics. Distance fields are one of these contemporary alternatives, and they offer more intuitive approaches to well-established techniques such as ambient occlusion and object shadows. <br>
This thesis gives an in-depth look into the most common distance field effects and presents a variety of acceleration techniques to show the viability of distance fields for modern game development.

Please refer to the [slides](talk_notes.pdf) from my talk at the University of Pennsylvania for an introduction into the topic of distance fields or my [thesis](Mroz_DistanceFields.pdf) for more in-depth information.

[![](Figures/vid.PNG)](https://youtu.be/ARlbxXxB1UQ)

Implementation
--------------
The entire code of the mTec renderer can be downloaded from this repo. Please note that Visual Studio and FreeGLUT/GLEW are required to compile an executable.
<br>
Keyboard Controls: <br>
q - quit <br>
Arrow Keys - Move <br>
v/space - Up/Down <br>
Mouse Left + Mouse Move - Camera Look <br>
w - recompile shaders <br>
e - toggle rotation <br>
r - toggle rotation speed <br>
a - toggle animation <br>
d - count average frametime <br>
s - toggle TAA <br>
 <br>
If the compilation of the project fails because of an GPP.exe error, please recompile a new [General PreProcessor](https://github.com/logological/gpp) exe and add it to the project.

![](Figures/6.PNG)
