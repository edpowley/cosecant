#pragma once

QString formatNote(double nn);

ERROR_CLASS(ParseNoteError);
double parseNote(const QString& noteString);

