#include "ScriptManager.h"
#include "core/Config.h"

ScriptManager::ScriptManager(QWidget *parent) :
    QWidget(parent), _field(0)
{
	QFont font;
	font.setPointSize(8);

	_groupScriptList = new GrpScriptList(this);
	_groupScriptList->setMinimumHeight(176);
	_groupScriptList->setFont(font);

	_scriptList = new ScriptList(this);
	_scriptList->setMaximumWidth(QFontMetrics(font).width(QString(20, 'M')));
	_scriptList->setMinimumHeight(88);
	_scriptList->setFont(font);

	_opcodeList = new OpcodeList(this);

	_compileScriptLabel = new QLabel(this);
	_compileScriptLabel->hide();
	QPalette pal = _compileScriptLabel->palette();
	pal.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
	pal.setColor(QPalette::Inactive, QPalette::WindowText, Qt::red);
	_compileScriptLabel->setPalette(pal);
	_compileScriptIcon = new QLabel(this);
	_compileScriptIcon->setPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical).pixmap(16));
	_compileScriptIcon->hide();
	
	QVBoxLayout *layoutGroupScript = new QVBoxLayout;
	layoutGroupScript->addWidget(_groupScriptList->toolBar());
	layoutGroupScript->addWidget(_groupScriptList);
	layoutGroupScript->setSpacing(2);
	layoutGroupScript->setContentsMargins(QMargins());

	QHBoxLayout *compileLayout = new QHBoxLayout;
	compileLayout->addWidget(_compileScriptIcon);
	compileLayout->addWidget(_compileScriptLabel, 1);
	compileLayout->setContentsMargins(QMargins());

	QVBoxLayout *layoutScript = new QVBoxLayout;
	layoutScript->addWidget(_opcodeList->toolBar());
	layoutScript->addWidget(_opcodeList);
	layoutScript->addLayout(compileLayout);
	layoutScript->setSpacing(2);
	layoutScript->setContentsMargins(QMargins());

	QGridLayout *contentLayout = new QGridLayout(this);
	contentLayout->addLayout(layoutGroupScript, 0, 0);
	contentLayout->addWidget(_scriptList, 0, 1);
	contentLayout->addLayout(layoutScript, 0, 2);
	contentLayout->setColumnStretch(0, 2);
	contentLayout->setColumnStretch(1, 1);
	contentLayout->setColumnStretch(2, 9);
	contentLayout->setContentsMargins(QMargins());

	connect(_groupScriptList, SIGNAL(changed()), SIGNAL(changed()));
	connect(_groupScriptList, SIGNAL(itemSelectionChanged()), SLOT(fillScripts()));

	connect(_scriptList, SIGNAL(itemSelectionChanged()), SLOT(fillOpcodes()));

	connect(_opcodeList, SIGNAL(changed()), SIGNAL(changed()));
	connect(_opcodeList, SIGNAL(changed()), SLOT(refresh()));
	connect(_opcodeList, SIGNAL(editText(int)), SIGNAL(editText(int)));
	connect(_opcodeList, SIGNAL(changed()), SLOT(compile()));

	_groupScriptList->toolBar()->setVisible(Config::value("grpToolbarVisible", true).toBool());
	_opcodeList->toolBar()->setVisible(Config::value("scriptToolbarVisible", true).toBool());

	addActions(QList<QAction *>()
	           << _groupScriptList->toolBar()->toggleViewAction()
	           << _opcodeList->toolBar()->toggleViewAction());
}

void ScriptManager::saveConfig()
{
	Config::setValue("grpToolbarVisible", !_groupScriptList->toolBar()->isHidden());
	Config::setValue("scriptToolbarVisible", !_opcodeList->toolBar()->isHidden());
}

void ScriptManager::clear()
{
	_groupScriptList->blockSignals(true);
	_scriptList->blockSignals(true);
	_opcodeList->blockSignals(true);

	_groupScriptList->clear();
	_groupScriptList->clearCopiedGroups();
	_groupScriptList->enableActions(false);
	_scriptList->clear();
	_opcodeList->clear();
	_opcodeList->clearCopiedOpcodes();
	_field = 0;

	_groupScriptList->blockSignals(false);
	_scriptList->blockSignals(false);
	_opcodeList->blockSignals(false);
}

void ScriptManager::fill(Field *field)
{
	_field = field;
	if(!_field) {
		return;
	}

	Section1File *scriptsAndTexts = _field->scriptsAndTexts();

	if (scriptsAndTexts->isOpen()) {
		_groupScriptList->blockSignals(true);
		_groupScriptList->fill(scriptsAndTexts);
		_groupScriptList->blockSignals(false);
		_groupScriptList->setEnabled(true);
	}
}

void ScriptManager::fillScripts()
{
	if(!_field) {
		return;
	}

	GrpScript *currentGrpScript = _groupScriptList->currentGrpScript();
	if (currentGrpScript) {
		_scriptList->blockSignals(true);
		_scriptList->fill(currentGrpScript);
		_scriptList->blockSignals(false);
		// Select first entry
		_scriptList->setCurrentRow(0);
	}
	emit groupScriptCurrentChanged(_groupScriptList->selectedID());
}

void ScriptManager::fillOpcodes()
{
	if(!_field) {
		return;
	}

	Script *currentScript = _scriptList->currentScript();
	if (currentScript) {
		_opcodeList->blockSignals(true);
		_opcodeList->fill(_field, _groupScriptList->currentGrpScript(),
		                 currentScript);
		_opcodeList->blockSignals(false);
		_opcodeList->setIsInit(_scriptList->selectedID() == 0);
		// Scroll to top
		_opcodeList->scroll(0, false);
	}
}

void ScriptManager::compile()
{
	Script *currentScript = _scriptList->currentScript();
	if(!currentScript) {
		return;
	}

	int opcodeID;
	QString errorStr;

	if(!currentScript->compile(opcodeID, errorStr)) {
		_compileScriptLabel->setText(tr("Error on line %1 : %2")
		                             .arg(opcodeID + 1)
		                             .arg(errorStr));
		_compileScriptLabel->show();
		_compileScriptIcon->show();
		_opcodeList->setErrorLine(opcodeID);
	} else {
		_compileScriptLabel->hide();
		_compileScriptIcon->hide();
		_opcodeList->setErrorLine(-1);
	}
}

void ScriptManager::refresh()
{
	_groupScriptList->localeRefresh();
	_scriptList->localeRefresh();
}

void ScriptManager::gotoOpcode(int grpScriptID, int scriptID, int opcodeID)
{
	_groupScriptList->scroll(grpScriptID, false);
	_scriptList->scroll(scriptID, false);
	_opcodeList->scroll(opcodeID);
}
