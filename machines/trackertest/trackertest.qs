function PatternEditor(machine, pattern)
{
	QGraphicsView.call(this);
	this.m_machine = machine;
	this.cscFunctions = machine.cscFunctions;
	this.m_pattern = pattern;
	
	this.m_font = new QFont("Courier", 12);
	var fontmetric = new QFontMetrics(this.m_font);
	this.m_fontWidth = fontmetric.averageCharWidth();
	this.m_fontHeight = fontmetric.lineSpacing();

	this.m_scene = new QGraphicsScene(this);
	this.setScene(this.m_scene);
	this.alignment = Qt.AlignLeft | Qt.AlignTop;
	
	var numrows = this.cscFunctions.getNumRows(this.m_pattern);
	var subdiv = this.cscFunctions.getSubdiv(this.m_pattern);
	var numtracks = this.cscFunctions.getNumTracks();
	
	this.m_cells = new Array(numrows);
	var cell;
	for (r=0; r<numrows; r++)
	{
		print("row", r, "of", numrows);
		this.m_cells[r] = new Array(numtracks);
		for (t=0; t<numtracks; t++)
		{
			cell = new Cell(this, r, t);
			this.m_cells[r][t] = cell;
			this.m_scene.addItem(cell);
			cell.setPos((t+1) * this.m_fontWidth * 7, (r+1) * this.m_fontHeight);
		}
		
		if (r % subdiv == 0)
		{
			label = new QGraphicsSimpleTextItem();
			label.setText((r/subdiv).toString());
			label.setFont(this.m_font);
			label.setPos(0, (r+1) * this.m_fontHeight);
			this.m_scene.addItem(label);
		}
	}
	
	this.m_cursor = new Cursor(this);
	this.m_scene.addItem(this.m_cursor);
}

PatternEditor.prototype = new QGraphicsView();

////////////////////////////////////////////////////////////////////

function Cursor(pe)
{
	QGraphicsRectItem.call(this, 0, 0, pe.m_fontWidth, pe.m_fontHeight);
	this.m_pe = pe;
	this.m_track = 0;
	this.m_column = 0;
	this.m_row = 0;
	this.updatePos();
}

Cursor.prototype = new QGraphicsRectItem();

Cursor.prototype.updatePos = function()
{
	var x = 7 + this.m_track*7 + this.m_column;
	var y = this.m_row + 1;
	this.setPos(x * this.m_pe.m_fontWidth, y * this.m_pe.m_fontHeight);
}

////////////////////////////////////////////////////////////////////

function Cell(pe, row, track)
{
	QGraphicsSimpleTextItem.call(this);
	this.m_pe = pe;
	this.m_row = row;
	this.m_track = track;
	
	this.setFont(this.m_pe.m_font);
	this.updateFromMachine();
}

Cell.prototype = new QGraphicsSimpleTextItem();

Cell.prototype.noteToStr = function(note)
{
	switch (note)
	{
	case -1:
		return "...";
	case -2:
		return "off";
	default:
		var octave = Math.floor(note / 12);
		var noten = note % 12;
		const notenames = "C-C#D-D#E-F-F#G-G#A-A#B-";
		if (octave >= 0 && octave <= 9)
			return notenames.substring(noten*2, noten*2+2) + octave.toString();
		else
			return "?!?";
	}
}

Cell.prototype.updateFromMachine = function()
{
	var celldata = this.m_pe.cscFunctions.getCell(this.m_pe.m_pattern, this.m_row, this.m_track);
	
	var notetext = this.noteToStr(celldata[0]);
	
	var vel = celldata[1];
	var veltext;
	if (vel == -1)
		veltext = "..";
	else if (vel >= 0 && vel <= 255)
		veltext = vel.toString(16).toUpperCase();
	else
		veltext = "?!";
	
	this.setText(notetext + " " + veltext);
}

PatternEditor.prototype.setCell = function(row, track, note, vel)
{
	this.cscFunctions.setCell(this.m_pattern, row, track, note, vel);
}

////////////////////////////////////////////////////////////////////////

// The main class
function TrackerTest()
{
}

TrackerTest.prototype.cscPatternEditor = PatternEditor;

////////////////////////////////////////////////////////////////////////

// Don't forget this! The script (or its last statement) should evaluate to your constructor
TrackerTest
