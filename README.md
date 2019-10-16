===============================================================<br/>
Summary of the project
https://drive.google.com/file/d/1oYEQtsip914irXk1wzTOyCY7x9lZYLMR/view"
===============================================================<br />
How to run the Simulator-Run-Configfile.py<br />
==============================================================<br />
-The Simulator-Run-Configfile.py is located in DPGAnalysis-SiPixelTools/HitAnalyzer/Simulator-Run-Configfile.py<br />
-Move to the src folder of the CMSSW folder<br />
-Run cmsenv<br />
-Run scram b -j <br />
-If scram b -j give you errors of missing library, try trace the error and deleting all files that cause the errors <br />
-In the Simulator-Run-Configfile.py put your Monte Carlo simulating .root data files at the line 32 <br />
-Make sure if your input files are in your local machine, use syntax file:path-to-file to configure your input files. <br />
-Run cmsRun with the Simulator-Run-Configfile.py <br />
-The output file will appear in the same folder as the config file and will be named raw.root <br />
-If running cmsRun results in an error regarding the version of the CMSSW, you have to upgrade your CMSSW framework.This is because the input files are created with a newer CMSSW framework. <br />
-Go to your home directory and run <br />
	cmsrel the-version-cmsRun-required<br />
-Go to github https://github.com/huankimtran/Pixel_Noise. Go to branch Simulator_Run_Configfile and download the DPGAnalysis-SiPixelTools folder to the src foler of the new CMSSW. <br />
-Go to the new CMSSW's src folder and redo everything from the first step. <br />
