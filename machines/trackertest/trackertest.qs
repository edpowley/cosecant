function PatternEditor(pattern)
{
	QLabel.call(this, "IOU one pattern editor");
	
	this.pattern = pattern;
}

PatternEditor.prototype = new QLabel();

////////////////////////////////////////////////////////////////////////

// The main class
function TrackerTest()
{
}

TrackerTest.prototype.cscPatternEditor = PatternEditor;

////////////////////////////////////////////////////////////////////////

// Don't forget this! The script (or its last statement) should evaluate to your constructor
TrackerTest
