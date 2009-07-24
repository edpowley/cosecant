#include "stdafx.h"
#include "common.h"
#include "patterneditor.h"
#include "machine.h"

PatternEditor::PatternEditor(const Ptr<Sequence::Pattern>& pattern)
: m_pattern(pattern)
{
	m_widget = m_pattern->m_mac->createPatternEditorWidget(pattern);
}

PatternEditor::~PatternEditor()
{
	if (m_pattern->m_editor == this)
		m_pattern->m_editor = NULL;
}

QString PatternEditor::getTitle()
{
	return QString(tr("Pattern - %1 - %2").arg(m_pattern->m_mac->getName()).arg(m_pattern->m_name));
}
