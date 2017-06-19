# Open Movement Summary Generator

## Tool

Download [omsummary.zip](omsummary.zip?raw=true) and extract the contents to a directory. 


## Sleep summary

From a source file `$DATASET.cwa`, you can produce a sleep analysis file `$DATASET.sleep.csv`. 

You must produce a text file `$DATASET.sleep.times.csv` with a particular format.  An easy way is to export a sheet from *Excel* to a *CSV file* (see secton below).  

The file must contain a first line (row) describing the columns (the line may be omitted if the order is unchanged, as this is the default):

	Start,End,Label

...and subsequent lines (rows) where date/times are formatted with the international standard `YYYY-MM-DD hh:mm:ss`, for example (where the label is the day):

	2015-12-05 01:00:00,2015-12-05 11:15:00,2015-12-05

Finally, drag the `$DATASET.sleep.csv` over the `omsummary-sleep.cmd` command script.  This will run `omsummary.exe` with the required parameters, and will generate the output file `$DATASET.sleep.summary.csv`.  This file (which can be opened in *Excel*), contains the headings:

	Label,Start,End,TimeInBed,SleepTime,SleepOnsetLatency,WakeTime,TimeToGetUp,WakeAfterSleepOnset,Awakenings,TotalSleepTime,SleepEfficiency

Where the times are all in minutes, and the columns are:

* Label: The label from the input `.times` file. 
* Start: The start time from the input `.times` file. 
* End: The end time from the input `.times` file. 
* TimeInBed: The *end*-*start* interval from the input `.times` file. 
* SleepTime: The actual start time of the first sleep period within the interval. 
* SleepOnsetLatency: The time after the start of the interval until the first sleep period within the interval. 
* WakeTime: The actual end time of the last sleep period within the interval. 
* TimeToGetUp: The time after the last sleep period within the interval, until the end of the interval. 
* WakeAfterSleepOnset: The elapsed time between the start time of the first sleep period, and the end time of the last sleep period, within the interval. 
* Awakenings: The number of sleep periods within the interval, minus one. 
* TotalSleepTime: The accumulated total of all of the sleep periods within the interval. 
* SleepEfficiency: The percentage of the total time in bed accounted as part of the total sleep time. 


### Exporting sleep times using Excel

This section describes the transformation from a sleep diary to the `sleep.times.csv` file that the summary tool requires. 

This is how the template `_TEMPLATE.sleep.times.xltx` file is configured.  You may use that file by copying in the sleep diary data in the first sheet, and exporting the second sheet as `.CSV` format and ensuring the file has the extensions ".sleep.times.csv".


Where the source format in `Sheet1` tab is:

	{date.day}/{date.month}/{date.year},{report.hour}:{report.min},*,*,{lof.hour},{lof.min},{lon.hour},{lon.min}

The sheet `csv` contains three columns, the first row:

	Start,End,Label

...and the second row the formulae

	Start	=Sheet1!$A2+IF(Sheet1!$E2*60+Sheet1!$F2>Sheet1!$G2*60+Sheet1!$H2,-1,0)+((Sheet1!$E2+(Sheet1!$F2/60))/24)
	End 	=Sheet1!$A2+((Sheet1!$G2+(Sheet1!$H2/60))/24)
	Label	=Sheet1!$A2+Sheet1!$B2

Which are then copied downwards.

Where the Start/End columns are formatted with:

	Format/number/custom/type: yyyy-mm-dd hh:mm:ss

...and the Label column is formatted with:

	Format/number/custom/type: yyyy-mm-dd

