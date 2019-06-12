Ambient lighting for monitor using Arduino and led strip. Screen is being continuously captured using DirectX from GPU (for maximum refresh rate) and then color values of individual LEDs are calculated and sent over serial to Arduino which controls the LED strip. This project is by no means finished, and it could use alot of refactoring and optimizations, however basic functionality can bee seen in video:

<a href="http://www.youtube.com/watch?feature=player_embedded&v=_Rl96Fl4sZs
" target="_blank"><img src="http://img.youtube.com/vi/_Rl96Fl4sZs/0.jpg" 
alt="Demo" width="240" height="180" border="10" /></a>

This project is based on [Microsoft's screen capture example using DXGI](https://code.msdn.microsoft.com/windowsdesktop/Desktop-Duplication-Sample-da4c696a) and therefore licesing in this project is bit more complex. Processing (.cpp, .h) are licensed under MIT license and only to this single file I (Igor Veres) own copyright. File SerialPort (.cpp, .h)
is licensed under MIT too, however I am not the author or copyright holder, I did however do minor changes to this file.
Copyright to other files on this project is held by Microsoft, and these files are licensed under MICROSOFT LIMITED PUBLIC LICENSE version 1.1 (provided as a file in this repo). The reason I am providing it, is because I added a function call into this sample code that calls my code.

## DISCLAIMER - please read!

This is a *personal* project and it is not inteded for easy use by other users. This repository may contain: hard-coded paths,
have no error handling, snippets lifted from stackoverflow or some other place, *__inappropriate__ error messages, comments and variable names*, generally disgusting code that
violates many good coding practices and there
is a slight chance it *might not even work correctly* - I am not guaranteeing anything. 

**The only reason this repository is public is for easy sharing with friends and hope that this code or snippet of it
might help somebody out there.**

With that said, feel free to ask questions and I will try to help you, or at least point you in the right direction.
