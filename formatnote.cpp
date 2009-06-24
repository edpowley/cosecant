#include "stdafx.h"
#include "common.h"
#include "formatnote.h"

const char* c_noteNames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};

QString formatNote(double nn)
{
	double whole, frac;
	frac = modf(nn, &whole);
	if (frac > 0.5)
	{
		whole += 1.0;
		frac  -= 1.0;
	}
	int note = (int)whole % 12;
	int oct  = (int)whole / 12;
	if (note < 0) note += 12;
	
	if (frac > 0)
		return QString("%1%2 + %3").arg(c_noteNames[note]).arg(oct).arg(frac);
	else if (frac < 0)
		return QString("%1%2 - %3").arg(c_noteNames[note]).arg(oct).arg(frac);
	else // frac == 0
		return QString("%1%2").arg(c_noteNames[note]).arg(oct);
}

double parseNote(const QString& noteString)
{
	QRegExp regex(
		"\\s*"							// whitespace
		"([A-Ga-g][-#])"				// [1] note name, followed by '-' or '#'
		"([-0-9][0-9]*)"				// [2] octave number
		"\\s*"							// whitespace
		"((?:[-+]\\s*[-+0-9.eE]*)?)"	// [3] optionally: + or -, whitespace, something which looks like a float
		"\\s*"							// whitespace
	);
	
	if (!regex.exactMatch(noteString)) throw ParseNoteError();

	int octave = regex.cap(2).toInt();
	QString notename = regex.cap(1).toUpper();
	int note = -1;
	for (int n=0; n<12 && note == -1; n++)
		if (notename == c_noteNames[n])
			note = n;
	if (note == -1)
		throw ParseNoteError();

	// Strip off the first character before atof'ing (may be whitespace between sign and number)
	QChar sign = regex.cap(3)[0];

	double frac = regex.cap(3).mid(1).toDouble();
	if (sign == '-') frac = -frac;

	return octave * 12 + note + frac;
}
