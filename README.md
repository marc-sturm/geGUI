# geGUI

geGUI is a generic graphical user interface for command-line applications. The program is implemented in C++ using the [Qt](http://qt-project.org/) library.

To generate a GUI for a tool, a command line interface definition in TDX format (see below) has to be provided. From this definition, a GUI is generated that can be used to execute the tool.<br><br>This screenshot shows an example of a tool for XML validation:

![Alt text](/doc/geGUI.png)

Features:

 * Free to use for everyone.
 * Portable Windows executable (does not require an installation).
 * Handling of input file, input file list, output file, string, integer, float, enum and flag parameters.
 * Handling of optional parameters and default values.


#Tool Definition XML (TDX)
TDX stands for the "Tool Definition XML" format. A TDX file describes the command line interface of a tool. It captures the tool documentation, various parameters, parameter documentation.
<br>
The XML schema of the TDX format can be found [here](/src/geGUI/Resources/TDX_v1.xsd).

By convention, the TDX file corresponding to an executable/script has the same file name with the appended extension '.tdx'.
