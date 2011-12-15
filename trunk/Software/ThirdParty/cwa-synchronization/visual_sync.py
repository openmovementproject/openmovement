#!/usr/bin/env python
# coding=UTF-8
#
#  CWA Data Synchronization Tool
#     
#  Copyright (c) 2011 Technische Universität München, 
#  Distributed Multimodal Information Processing Group
#  http://vmi.lmt.ei.tum.de
#  All rights reserved.
#
#  Stefan Diewald <stefan.diewald [at] tum.de>
#     
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of the University or Institute nor the names
#    of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import datetime
from pylab import *
import matplotlib
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure
from numpy import convolve # for cross correlation
import sqlite3

class Spike:
    """
    Data-structure for storing a single spike/shaking.
    """
    def __init__(self, start = 0):
        self.start = start
        self.end = start
        self.count = 0
        
class SpikeMatch:
    pass

class cwa_database:
    def __init__(self, db_filename):
        # sqlite connect to given sqlite database
        self.connection = sqlite3.connect(db_filename)
        # add python's sqrt and pow function to sqlite database
        self.connection.create_function("sqrt", 1, sqrt)
        self.connection.create_function("pow", 2, pow)
        self.cur = self.connection.cursor()
        # this exception handler is used for checking whether there is a valid CWA database or not
        try:
            # check if table acc exists, will raise sqlite3.DatabaseError if table is unknown,
            # or the opened file is not a valid database
            cols = self.cur.execute("PRAGMA table_info(acc);").fetchall()
            if not len(cols) > 0:
                self.connection = None
                self.cur = None
                raise sqlite3.DatabaseError
        except sqlite3.DatabaseError:
            self.connection = None
            self.cur = None
            raise sqlite3.DatabaseError
        
        # get the available column names in acc (from "PRAGMA table_info(acc);" query)
        self.cols = [col[1] for col in cols]
        
    def execute(self, sql):
        self.cur.execute(sql)
        
    def get_colums(self, cols):
        lcols = len(cols)
        data = []
        icol = 0
        for col in cols:
            data[icol] = []
            icol += 1
        icol = 0
        for row in self.cur:
            icol = 0
            for col in cols:
                data[icol] += [row[col]]
                icol += 1
        return data
    
    def getBetweenDates(self, start, end):
        self.cur.execute("SELECT * FROM acc WHERE time between ? and ? ORDER BY time ASC;", (start, end))
        return self.cur
    
    def getSampleCountBetweenDates(self, start, end):
        self.cur.execute("SELECT count(*) FROM acc WHERE time between ? and ?;", (start, end))
        return self.cur.fetchone()[0]
    
    def getAmplitudeBetweenDates(self, start, end):
        """
        Get the magnitude of every sample between the given timestamps.
        
        Magnitude = absolute value of acceleration vector = sqrt( x^2 + y^2 + z^2 )
        """
        self.cur.execute("SELECT sqrt(pow(x,2) + pow(y,2) + pow(z,2)) as amplitude FROM acc WHERE time between ? and ? ORDER BY time ASC;", (start, end))
        amps = []
        for row in self.cur:
            amps += [ row[0] ]
        return amps
    
    def getSpikes(self, min_power = 1200, min_count = 20):
        """
        Algorithm for finding shakings:
         - Find acceleration values that exceed min_power
         - Count the number of contiguous spikes that are less than 300 ms apart from each other
         - If this number is higher than min_count => assume this values stem from shaking
        """
        self.cur.execute("SELECT time FROM acc WHERE abs(x) > ? or abs(y) > ? or abs(z) > ? ORDER BY time ASC;", (min_power, min_power, min_power))
        
        s = Spike()
        spikes = []
        for row in self.cur:
            date = row[0]
            # if the last value exceeding min_power is more than 300 ms away
            if date - s.end > 0.3:
                # check if the count transcends the min_count threshold
                if s.count > min_count:
                    # add spike to spike list
                    spikes += [ s ]
                s = Spike(date)
            else:
                # the values with min_power are less apart than 300 ms => count it and reset time
                s.count += 1
                s.end = date
        if s.count > min_count:
            spikes += [ s ]
        
        return spikes

def matchSpikes(db1, db2):
    """
    Returns a dict with the best matching spikes.
    """
    # get spikes with at least 30 contiguous min. axis amplitudes of 1200 mg (milli-gravity) 
    spikes1 = db1.getSpikes(1200, 30)
    spikes2 = db2.getSpikes(1200, 30)
    
    smatches = []
    timeshifts = {}
    
    is1 = -1
    # for all found "shakings" in database 1
    for s1 in spikes1:
        is1 += 1
        # get magnitude = sqrt(sum(pow(x,2))) samples belonging to the shaking
        amps1 = db1.getAmplitudeBetweenDates(s1.start, s1.end)
        is2 = -1
        # for all found "shakings" in database 2
        for s2 in spikes2:
            is2 += 1
            # similarity is only given, when the shakings have almost equal length (+/- 15 %)
            # and have similar length (+/- 0.5 s)
            if abs(s1.count - s2.count) < 0.15 * (s1.count + s2.count) / 2 \
            and abs((s1.end - s1.start) - (s2.end - s2.start)) < 0.5:
                # get magnitude = sqrt(sum(pow(x,2))) samples belonging to the shaking
                amps2 = db2.getAmplitudeBetweenDates(s2.start, s2.end)
                """ 
                Calculate cross-correlation
                
                Cross-correlation can be used to measure the similarity of two functions.
                It is used here for the verification that two detected shakings are similar
                and not for detecting the time lag between those.
                    
                Cross-correlation is similar to convolution with a none-reversed signal.
                In order to use NumPy's convolve function for the correlation, one signal is
                passed reversed to this functions. As the convolve function automatically reverses
                one of the passed vectors, this calculation corresponds to cross correlation.
                
                Convolution = Reverse a signal, shift it and multiply with another signal
                Cross correlation = Shift a signal and multiply with another signal
                
                Discrete correlation example:
                s1 = [ 1 -3 5 -2 1 ]
                s2 = [ 0 -3 4 -2 1 ]
                
                s1  s2  s2[0]  s2[1]  s2[1]  s2[2]  s2[3]
                 1   0    0      0      0      4      1
                 5   4    0      0      4      1      0
                 1   1    0      4      1      0      0
                 xcorr    0      4     21      9      1
                """
                xcorr = convolve(amps1, amps2[::-1])

                # calculate average time shift for re-alignment
                timeshift = ((s1.start + s1.end) - (s2.start + s2.end)) / 2.0
                
                # make sure, cross-correlation has maximum peak near centre (5% deviation allowed)
                if abs(list(xcorr).index(max(xcorr)) - len(xcorr) / 2) < ceil(0.05 * len(xcorr)):
                    smatch = SpikeMatch()
                    # store the spike from file 1
                    smatch.s1 = s1
                    # store the spike identifier from file 1
                    smatch.is1 = is1
                    # store the spike from file 2
                    smatch.s2 = s2
                    # store the spike identifier from file 2
                    smatch.is2 = is2
                    # store the maximum correlation
                    smatch.xcorr = max(xcorr)
                    # store the average timeshift
                    smatch.timeshift = timeshift
                    smatches += [smatch]
                    
                    # count the number of different timeshifts
                    added_shift = False
                    for tshift in dict.keys(timeshifts):
                        if abs(smatch.timeshift - tshift) < 1.0:
                            timeshifts[tshift] += 1
                            added_shift = True
                            break
                    
                    # add timeshift to list if not found in the count routine
                    if added_shift == False:
                        timeshifts[smatch.timeshift] = 1
             
    smatches2 = []
    # only spike groups with timeshifts that occur several times are considered
    for tshift in dict.keys(timeshifts):
        if timeshifts[tshift] > 1:
            sid = 0
            for smatch in smatches:
                if abs(smatch.timeshift - tshift) < 1.0:
                    smatches2 += [smatch]
                    
    return smatches2

class ListModel(QAbstractListModel):
    """
    ListModel holds the data for ListView.
    """ 
    def __init__(self, data_in, parent=None, *args): 
        QAbstractListModel.__init__(self, parent, *args) 
        self.listdata = data_in
 
    def rowCount(self, parent=QModelIndex()): 
        return len(self.listdata)
    
    def clear(self):
        """
        Clear all list entries.
        """
        self.listdata = []
        # force redraw
        self.reset()
    
    def appendData(self, data_in):
        """
        Append entry to list
        """
        self.listdata += [data_in]
        # force redraw
        self.reset()
 
    def data(self, index, role):
        """
        Return the data for the ListView
        """
        if index.isValid() and role == Qt.DisplayRole:
            return QVariant(self.listdata[index.row()])
        else: 
            return QVariant()

class AppForm(QMainWindow):
    """
    PyQt4 Application
    """
    def __init__(self, parent=None):
        """
        Create Window with menu, status bar and main frame
        """
        QMainWindow.__init__(self, parent)
        self.setWindowTitle('CWA Data Synchronization')

        # init database handler
        self.db1 = None
        self.db2 = None
        
        # init match handler
        self.smatch1 = None
        self.smatch2 = None

        self.create_menu()
        self.create_main_frame()
        self.create_status_bar()

    def save_plot(self):
        file_choices = "PNG (*.png)|*.png"
        
        path = unicode(QFileDialog.getSaveFileName(self, 
                        'Save file', '', 
                        file_choices))
        if path:
            self.canvas.print_figure(path, dpi=self.dpi)
            self.statusBar().showMessage('Saved to %s' % path, 2000)
    
    def on_about(self):
        msg = """CWA Data Synchronization Tool
        
        Copyright (c) 2011 Technische Universität München,
        Distributed Multimodal Information Processing Group
        http://vmi.lmt.ei.tum.de
        
        Stefan Diewald <stefan.diewald@tum.de>
        """
        QMessageBox.about(self, "About CWA Data Synchronization", self.trUtf8(msg.strip()))
        
    def getMatches(self):
        """
        Get the matching shakings and add them to the ListViews.
        
        The starting point list will show all matches except the last one.
        The end point list will show all matches except the first one.
        """
        self.smatches = matchSpikes(self.db1, self.db2)
        # clear the list data
        self.lm1.clear()
        self.lm2.clear()
        self.axes.clear()
        self.canvas.draw()
        i = 0
        for smatch in self.smatches:
            matchstr = str(datetime.datetime.fromtimestamp(smatch.s1.start).strftime("%H:%M:%S")) + " and " + str(datetime.datetime.fromtimestamp(smatch.s2.start).strftime("%H:%M:%S")) + " match (timeshift: " + str(smatch.timeshift) + " s)"
            if (i != len(self.smatches) - 1):
                # do not add the last entry to the start point list
                self.lm1.appendData(matchstr)
            if (i > 0):
                # exclude the first match from the end point list
                self.lm2.appendData(matchstr)
            i += 1
        if i == 2:
            # if there are only two matches, try to get the matched graph
            self.getMatchedGraph()
        
        # select the first entries in every list
        self.list_view1.selectionModel().select(QItemSelection(self.lm1.index(0),self.lm1.index(0)),QItemSelectionModel.Select)
        self.list_view2.selectionModel().select(QItemSelection(self.lm2.index(0),self.lm2.index(0)),QItemSelectionModel.Select)
        
        self.find_matches_label.setText("Found " + str(len(self.smatches)) + " match(es).")
        self.statusBar().showMessage('Found %d match(es).' % (len(self.smatches)))
    
    def getMatchedGraph(self):
        """
        Show a preview of the selected data in the preview field.
        """
        # only allow different shakings for start- and end-point
        if not self.smatch1 == None and not self.smatch2 == None \
        and not self.smatch1.is1 == self.smatch2.is1 and not self.smatch1.is2 == self.smatch2.is2:
            # get start and end point in correct order (interchange if necessary)
            if self.smatch1.s1.start > self.smatch2.s1.start:
                temp = self.smatch1
                self.smatch1 = self.smatch2
                self.smatch2 = temp
                
            # calculate the average timeshift that will be used for correction
            correct_timeshift = (self.smatch1.timeshift + self.smatch2.timeshift) / 2.0
            
            # get the number of samples in the period
            count1 = self.db1.getSampleCountBetweenDates(self.smatch1.s1.start, self.smatch2.s1.end)
            count2 = self.db2.getSampleCountBetweenDates(self.smatch1.s2.start, self.smatch2.s2.end)
            
            # request all the samples between the start and end point
            sync_data1 = self.db1.getBetweenDates(self.smatch1.s1.start, self.smatch2.s1.end)
            sync_data2 = self.db2.getBetweenDates(self.smatch1.s2.start, self.smatch2.s2.end)
            
            # if there are less than 1,000,000 samples, they can be displayed in the plot
            if count1 < 1000000 and count2 < 1000000:
                data1 = [0, 1, 2, 3]
                data1[0] = []; data1[1] = []; data1[2] = []; data1[3] = []
                for row in sync_data1:
                    data1[0] += [row[0]]
                    if self.x_acc_cb.isChecked():
                        data1[1] += [row[1] / 256.0]
                    if self.y_acc_cb.isChecked():
                        data1[2] += [row[2] / 256.0]
                    if self.z_acc_cb.isChecked():
                        data1[3] += [row[3] / 256.0]
                data2 = [0, 1, 2, 3]
                data2[0] = []; data2[1] = []; data2[2] = []; data2[3] = []
                for row in sync_data2:
                    # correct with timeshift
                    data2[0] += [(row[0] + correct_timeshift)]
                    if self.x_acc_cb.isChecked():
                        data2[1] += [row[1] / 256.0]
                    if self.y_acc_cb.isChecked():
                        data2[2] += [row[2] / 256.0]
                    if self.z_acc_cb.isChecked():
                        data2[3] += [row[3] / 256.0]
                    
                self.axes.clear()
                
                graphs = ()
                legends = ()
                
                # get the filename without file extension 
                filename1 = os.path.splitext(os.path.basename(self.filename1))[0]
                filename2 = os.path.splitext(os.path.basename(self.filename2))[0]
                
                if self.x_acc_cb.isChecked():
                    p1 = self.axes.plot(data1[0], data1[1], '-', marker = ',')
                    p2 = self.axes.plot(data2[0], data2[1], '-', marker = ',')
                    graphs += (p1, p2)
                    legends += (filename1 + ' x', filename2 + ' x')
                    
                if self.y_acc_cb.isChecked():
                    p3 = self.axes.plot(data1[0], data1[2], '-', marker = ',')
                    p4 = self.axes.plot(data2[0], data2[2], '-', marker = ',')
                    graphs += (p3, p4)
                    legends += (filename1 + ' y', filename2 + ' y')
                    
                if self.z_acc_cb.isChecked():
                    p5 = self.axes.plot(data1[0], data1[3], '-', marker = ',')
                    p6 = self.axes.plot(data2[0], data2[3], '-', marker = ',')
                    graphs += (p5, p6)
                    legends += (filename1 + ' z', filename2 + ' z')
                    
                if self.battery_cb.isChecked():
                    pass
                
                if self.temperature_cb.isChecked():
                    pass
                
                if self.light_cb.isChecked():
                    pass
                
                if len(graphs) > 0:
                    # add a legend
                    self.axes.legend(graphs, legends, 'best')
                
                # draw the changed axes
                self.canvas.draw()
            else:
                self.statusBar().showMessage('Too many samples for preview: %d.' % (count1 + count2))
                    
    def open1(self):
        (self.filename1, self.db1) = self.open(self.load1_label)
        self.checkDB()
                
    def open2(self):
        (self.filename2, self.db2) = self.open(self.load2_label)
        self.checkDB()
        
    def open(self, label):
        """
        Open and load database files.
        """
        filename = str(QFileDialog.getOpenFileName(self, 'Open CWA-SQLite File', '.sqlite;;.'))
        self.axes.clear()
        try:
            db = cwa_database(filename)
            label.setText(os.path.basename(filename))
            self.statusBar().showMessage('Successfully loaded %s.' % (filename))
        except sqlite3.DatabaseError:
            if len(filename.strip()) > 0:
                QMessageBox.critical(self,
                        "Database Error",
                        "%s is not a valid CWA database!" % (os.path.basename(filename))
                )
            label.setText("")
            db = None
        return (filename, db)
            
    def checkDB(self):
        """
        Check if there are two valid databases opened.
        """
        self.lm1.clear()
        self.lm2.clear()
        if self.db1 != None and self.db2 != None:
            self.find_matches_button.setEnabled(True)
            self.checkAvailableData()
            self.getMatches()
        else:
            self.find_matches_button.setEnabled(False)
            self.find_matches_label.setText("")
            
    def checkAvailableData(self):
        """
        Disable all check boxes where no data is available.
        """
        dependencies = [
            ['x', (self.x_acc_cb, self.x_acc_exp)],
            ['y', (self.y_acc_cb, self.y_acc_exp)],
            ['z', (self.z_acc_cb, self.z_acc_exp)],
            ['light', (self.light_cb, self.light_exp)],
            ['battery', (self.battery_cb, self.battery_exp)],
            ['temperature', (self.temperature_cb, self.temperature_exp)]
        ]
        
        for dep in dependencies:
            if dep[0] in self.db1.cols and dep[0] in self.db2.cols:
                for cbox in dep[1]:
                    cbox.setEnabled(True)
            else:
                for cbox in dep[1]:
                    cbox.setChecked(False)
                    cbox.setEnabled(False)  
            
    def export_data(self):
        """
        Export synchronized data to various file format:
         - currently only csv is supported.
        """
        # only allow different shakings for start- and end-point
        if not self.smatch1 == None and not self.smatch2 == None \
        and not self.smatch1.is1 == self.smatch2.is1 and not self.smatch1.is2 == self.smatch2.is2:
            # get the filename without file extension and add timestamp from the first data 
            filename1 = os.path.splitext(os.path.basename(self.filename1))[0] + '_' + str(self.smatch1.s1.start)
            filename2 = os.path.splitext(os.path.basename(self.filename2))[0] + '_' + str(self.smatch1.s1.start)
            
            # calculate the time-lag between the matches
            correct_timeshift = (self.smatch1.timeshift + self.smatch2.timeshift) / 2.0
            
            # get the number of samples in the period
            count1 = self.db1.getSampleCountBetweenDates(self.smatch1.s1.start, self.smatch2.s1.end)
            count2 = self.db2.getSampleCountBetweenDates(self.smatch1.s2.start, self.smatch2.s2.end)
            
            # calculate the duration of a single timestep
            timesteps1 = (self.smatch2.s1.end - self.smatch1.s1.start) / (count1 - 1)
            timesteps2 = (self.smatch2.s1.end - self.smatch1.s1.start) / (count2 - 1)
            
            # get the data from the database
            sync_data1 = self.db1.getBetweenDates(self.smatch1.s1.start, self.smatch2.s1.end)
            sync_data2 = self.db2.getBetweenDates(self.smatch1.s2.start, self.smatch2.s2.end)
            
            # export the data for both files
            for data in ((sync_data1, count1, timesteps1, filename1) , (sync_data2, count2, timesteps2, filename2)):
                save_name = QFileDialog.getSaveFileName(self, "Save file " + data[3], data[3] + '.csv', ".csv")
                j = -1
                f = open(save_name, 'w')
                for row in data[0]:
                    j += 1
                    t = j * data[2]
                    f.write(str(t))
                    if self.x_acc_exp.isChecked():
                        f.write(',' + str(row[1] / 256.0))
                    if self.y_acc_exp.isChecked():
                        f.write(',' + str(row[2] / 256.0))
                    if self.y_acc_exp.isChecked():
                        f.write(',' + str(row[3] / 256.0))
                    if self.light_exp.isChecked():
                        f.write(',' + str(row[self.db1.cols.index('light')]))
                    if self.battery_exp.isChecked():
                        f.write(',' + str(row[self.db1.cols.index('battery')]))
                    if self.battery_exp.isChecked():
                        f.write(',' + str(row[self.db1.cols.index('temperature')]))
                    f.write('\n')
                f.close()
            self.statusBar().showMessage('Export successful.')
    
    def create_main_frame(self):
        """
        """
        
        self.main_frame = QWidget()
        
        # create the matplotlib figure
        self.dpi = 100
        # initial figure size in inch
        self.fig = Figure((9.0, 5.0), dpi=self.dpi)
        self.canvas = FigureCanvas(self.fig)
        self.canvas.setParent(self.main_frame)
        
        # using subplot in order to get the navigation toolbar working
        self.axes = self.fig.add_subplot(111)
        # create the navigation toolbar
        self.mpl_toolbar = NavigationToolbar(self.canvas, self.main_frame)
        
        load_label = QLabel()
        load_label.setText("Load CWA database files:")
        
        self.load1_button = QPushButton("Load CWA-SQLite &1")
        self.connect(self.load1_button, SIGNAL('clicked()'), self.open1)
        self.load1_label = QLabel()
        
        self.load2_button = QPushButton("Load CWA-SQLite &2")
        self.connect(self.load2_button, SIGNAL('clicked()'), self.open2)
        self.load2_label = QLabel()
         
        self.find_matches_button = QPushButton("&Find matches")
        self.find_matches_button.setEnabled(False)
        self.connect(self.find_matches_button, SIGNAL('clicked()'), self.getMatches)
        self.find_matches_label = QLabel()
        
        self.x_acc_cb = QCheckBox("&x-acceleration")
        self.x_acc_cb.setChecked(True)
        self.connect(self.x_acc_cb, SIGNAL('stateChanged(int)'), self.getMatchedGraph)
        
        self.y_acc_cb = QCheckBox("&y-acceleration")
        self.connect(self.y_acc_cb, SIGNAL('stateChanged(int)'), self.getMatchedGraph)
        
        self.z_acc_cb = QCheckBox("&z-acceleration")
        self.connect(self.z_acc_cb, SIGNAL('stateChanged(int)'), self.getMatchedGraph)
        
        self.light_cb = QCheckBox("&light")
        self.connect(self.light_cb, SIGNAL('stateChanged(int)'), self.getMatchedGraph)
        
        self.temperature_cb = QCheckBox("&temperature")
        self.connect(self.temperature_cb, SIGNAL('stateChanged(int)'), self.getMatchedGraph)
        
        self.battery_cb = QCheckBox("&battery")
        self.connect(self.battery_cb, SIGNAL('stateChanged(int)'), self.getMatchedGraph)
        
        self.x_acc_exp = QCheckBox("x-acceleration")
        self.y_acc_exp = QCheckBox("y-acceleration")
        self.z_acc_exp = QCheckBox("z-acceleration")
        self.light_exp = QCheckBox("light")
        self.temperature_exp = QCheckBox("temperature")
        self.battery_exp = QCheckBox("battery")
        
        self.export_button = QPushButton("&Export data")
        self.connect(self.export_button, SIGNAL('clicked()'), self.export_data)
        
        #
        # Layout with box sizers
        # 
        hbox_load = QHBoxLayout()
        vbox_load1 = QVBoxLayout()
        vbox_load1.addWidget(self.load1_button)
        vbox_load1.addWidget(self.load1_label)
        hbox_load.addLayout(vbox_load1)
        vbox_load2 = QVBoxLayout()
        vbox_load2.addWidget(self.load2_button)
        vbox_load2.addWidget(self.load2_label)
        hbox_load.addLayout(vbox_load2)
        vbox_load3 = QVBoxLayout()
        vbox_load3.addWidget(self.find_matches_button)
        vbox_load3.addWidget(self.find_matches_label)
        hbox_load.addLayout(vbox_load3)
                   
        self.start_label = QLabel("Start point:")
        self.lm1 = ListModel([], self)
        self.list_view1 = QListView()
        self.list_view1.setModel(self.lm1)
        self.connect(self.list_view1.selectionModel(), SIGNAL("selectionChanged(QItemSelection, QItemSelection)"), self.slot_match1)
        
        self.end_label = QLabel("End point:")
        self.lm2 = ListModel([], self)
        self.list_view2 = QListView()
        self.list_view2.setModel(self.lm2)
        self.connect(self.list_view2.selectionModel(), SIGNAL("selectionChanged(QItemSelection, QItemSelection)"), self.slot_match2)
        
        hbox_matches = QHBoxLayout()
        vbox_matches1 = QVBoxLayout()
        vbox_matches1.addWidget(self.start_label)
        vbox_matches1.addWidget(self.list_view1)
        hbox_matches.addLayout(vbox_matches1)
        vbox_matches2 = QVBoxLayout()
        vbox_matches2.addWidget(self.end_label)
        vbox_matches2.addWidget(self.list_view2)
        hbox_matches.addLayout(vbox_matches2)
        
        hbox_preview = QHBoxLayout()
        for w in [ self.x_acc_cb, self.y_acc_cb, self.z_acc_cb, self.battery_cb, self.temperature_cb, self.light_cb  ]:
            hbox_preview.addWidget(w)
            hbox_preview.setAlignment(w, Qt.AlignVCenter)
        
        # group box for preview data
        self.groupBox_preview = QGroupBox('Show data in preview')
        self.groupBox_preview.setLayout(hbox_preview)
        
        # export layout
        vbox_export = QVBoxLayout()
        hbox_export_axis = QHBoxLayout()
        for w in [ self.x_acc_exp, self.y_acc_exp, self.z_acc_exp, self.battery_exp, self.temperature_exp, self.light_exp ]:
            hbox_export_axis.addWidget(w)
            hbox_export_axis.setAlignment(w, Qt.AlignVCenter)
        vbox_export.addLayout(hbox_export_axis)
        vbox_export.addWidget(self.export_button)
        
        # groupbox for the export things
        self.groupBox_export = QGroupBox('Export synchronized data')
        self.groupBox_export.setLayout(vbox_export)
        
        # the main vertical layout
        vbox_main = QVBoxLayout()
        vbox_main.addWidget(load_label)
        vbox_main.addLayout(hbox_load)
        vbox_main.addLayout(hbox_matches)
        vbox_main.addWidget(self.canvas)
        vbox_main.addWidget(self.mpl_toolbar)
        vbox_main.addWidget(self.groupBox_preview)
        vbox_main.addWidget(self.groupBox_export)
        
        self.main_frame.setLayout(vbox_main)
        self.setCentralWidget(self.main_frame)
    
    def slot_match1(self, new_selection, old_selection):
        """
        Called on selection change in start point list.
        """
        for index in new_selection.indexes():
            self.smatch1 = self.smatches[index.row()]
            self.getMatchedGraph()
            
    def slot_match2(self, new_selection, old_selection):
        """
        Called on selection change in end point list.
        """
        for index in new_selection.indexes():
            self.smatch2 = self.smatches[index.row()+1]
            self.getMatchedGraph()
    
    def create_status_bar(self):
        self.status_text = QLabel("Select CWA database files.")
        self.statusBar().addWidget(self.status_text, 1)
        
    def create_menu(self):
        # create File menu entry
        self.file_menu = self.menuBar().addMenu("&File")
        load_file_action = self.create_action("&Save plot", shortcut="Ctrl+S", slot=self.save_plot, tip="Save the plot")
        quit_action = self.create_action("&Quit", slot=self.close, shortcut="Ctrl+Q", tip="Close the application")
        self.add_actions(self.file_menu, (load_file_action, None, quit_action))
        
        # create Help menu entry
        self.help_menu = self.menuBar().addMenu("&Help")
        about_action = self.create_action("&About", shortcut='F1', slot=self.on_about, tip='About the demo')
        self.add_actions(self.help_menu, (about_action,))

    def add_actions(self, target, actions):
        """
        Add the QActions to the menu.
        """
        for action in actions:
            if action is None:
                target.addSeparator()
            else:
                target.addAction(action)

    def create_action(self, text, slot=None, shortcut=None, icon=None, tip=None, checkable=False, signal="triggered()"):
        """
        Create actions for the menu bar items.
        """
        action = QAction(text, self)
        if icon is not None:
            action.setIcon(QIcon(":/%s.png" % icon))
        if shortcut is not None:
            action.setShortcut(shortcut)
        if tip is not None:
            action.setToolTip(tip)
            action.setStatusTip(tip)
        if slot is not None:
            self.connect(action, SIGNAL(signal), slot)
        if checkable:
            action.setCheckable(True)
        return action

def main():
    app = QApplication([])
    form = AppForm()
    form.show()
    app.exec_()

if __name__ == "__main__":
    main()