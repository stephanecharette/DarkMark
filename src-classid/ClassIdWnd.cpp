// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::ClassIdWnd::ClassIdWnd(const std::string & fn) :
	DocumentWindow("DarkMark - " + File(fn).getFileName(), Colours::darkgrey, TitleBarButtons::closeButton),
	ThreadWithProgressWindow("DarkMark", true, true),
	names_fn				(fn),
	add_button				("Add Row"),
	apply_button			("Apply..."),
	cancel_button			("Cancel")
{
	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	canvas.addAndMakeVisible(table);
	canvas.addAndMakeVisible(add_button);
	canvas.addAndMakeVisible(apply_button);
	canvas.addAndMakeVisible(cancel_button);

	table.getHeader().addColumn("original id"	, 1, 100, 30, -1, TableHeaderComponent::notSortable);
	table.getHeader().addColumn("original name"	, 2, 100, 30, -1, TableHeaderComponent::notSortable);
	table.getHeader().addColumn("delete"		, 3, 100, 30, -1, TableHeaderComponent::notSortable);
	table.getHeader().addColumn("merge into"	, 4, 100, 30, -1, TableHeaderComponent::notSortable);
	table.getHeader().addColumn("new id"		, 5, 100, 30, -1, TableHeaderComponent::notSortable);
	table.getHeader().addColumn("new name"		, 6, 100, 30, -1, TableHeaderComponent::notSortable);

	table.getHeader().setStretchToFitActive(true);
	table.getHeader().setPopupMenuActive(false);
	table.setModel(this);

	add_button		.addListener(this);
	apply_button	.addListener(this);
	cancel_button	.addListener(this);

	std::ifstream ifs(names_fn);
	std::string line;
	while (std::getline(ifs, line))
	{
		add_row(line);
	}

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("ClassIdWnd"))
	{
		restoreWindowStateFromString(cfg().getValue("ClassIdWnd"));
	}
	else
	{
		centreWithSize(640, 400);
	}

	rebuild_table();

	setVisible(true);

	return;
}


dm::ClassIdWnd::~ClassIdWnd()
{
	signalThreadShouldExit();
	cfg().setValue("ClassIdWnd", getWindowStateAsString());

	return;
}


void dm::ClassIdWnd::add_row(const std::string & name)
{
	const int row		= vinfo.size();

	Info info;
	info.original_id	= row;
	info.original_name	= name;
	info.must_delete	= false;
	info.merge_into_id	= -1;
	info.modified_id	= -1;
	info.modified_name	= name;

	vinfo.push_back(info);

	auto button = new ToggleButton();
	button->setComponentID(String(row) + "_MUST_DELETE");
	button->setLookAndFeel(&button_look_and_feel);
	button->setToggleable(true);
	button->setClickingTogglesState(true);
	button->addListener(this);
	toggle_buttons.push_back(button);

	auto combobox = new ComboBox();
	combobox->setComponentID(String(row) + "_MERGE_INTO");
	combobox->addListener(this);
	combo_boxes.push_back(combobox);

	return;
}


void dm::ClassIdWnd::closeButtonPressed()
{
	setVisible(false);
	exitModalState(0);

	return;
}


void dm::ClassIdWnd::userTriedToCloseWindow()
{
	// ALT+F4

	closeButtonPressed();

	return;
}


void dm::ClassIdWnd::resized()
{
	// get the document window to resize the canvas, then we'll deal with the rest of the components
	DocumentWindow::resized();

	const int margin_size = 5;

	FlexBox button_row;
	button_row.flexDirection = FlexBox::Direction::row;
	button_row.justifyContent = FlexBox::JustifyContent::flexEnd;
	button_row.items.add(FlexItem(add_button)		.withWidth(100.0));
	button_row.items.add(FlexItem()					.withFlex(1.0));
	button_row.items.add(FlexItem(apply_button)		.withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
	button_row.items.add(FlexItem(cancel_button)	.withWidth(100.0));

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.items.add(FlexItem(table).withFlex(1.0));
	fb.items.add(FlexItem(button_row).withHeight(30.0).withMargin(FlexItem::Margin(margin_size, 0, 0, 0)));

	auto r = getLocalBounds();
	r.reduce(margin_size, margin_size);
	fb.performLayout(r);

	return;
}


void dm::ClassIdWnd::buttonClicked(Button * button)
{
	if (button == &add_button)
	{
		add_row("new class");
		table.updateContent();
	}
	else if (button == &cancel_button)
	{
		closeButtonPressed();
	}
	else
	{
		const String id = button->getComponentID();
		if (id.endsWith("_MUST_DELETE"))
		{
			const int row = id.getIntValue();
			const bool must_delete = button->getToggleState();
			vinfo[row].must_delete = must_delete;
			if (must_delete)
			{
				// if we're deleting this class, then that means we're not merging it into anything
				vinfo[row].merge_into_id = -1;
				combo_boxes[row]->clear();
			}
			combo_boxes[row]->setEnabled(not must_delete);
			rebuild_table();
		}
	}

	return;
}


void dm::ClassIdWnd::comboBoxChanged(ComboBox * comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged == nullptr)
	{
		return;
	}

	const String id = comboBoxThatHasChanged->getComponentID();
	if (id.endsWith("_MERGE_INTO"))
	{
		const int row = id.getIntValue();
		const auto text = comboBoxThatHasChanged->getText();

		// find the user-selected text in the vector and remember the original ID so we know exactly what needs to be merged
		int merge_id = -1;
		for (const auto & info : vinfo)
		{
			if (info.modified_name == text)
			{
				merge_id = info.original_id;
				break;
			}
		}

		vinfo[row].merge_into_id	= merge_id;
		vinfo[row].modified_id		= -1;
		vinfo[row].modified_name	= "";

		rebuild_table();
	}

	return;
}


void dm::ClassIdWnd::run()
{
}


int dm::ClassIdWnd::getNumRows()
{
	return vinfo.size();
}


void dm::ClassIdWnd::paintRowBackground(Graphics &g, int rowNumber, int width, int height, bool rowIsSelected)
{
	if (rowNumber < 0					or
		rowNumber >= (int)vinfo.size()	)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	Colour colour = Colours::white;
	if (rowIsSelected)
	{
		colour = Colours::lightblue; // selected rows will have a blue background
	}
	else if (vinfo[rowNumber].must_delete)
	{
		colour = Colours::darksalmon;
	}
	else
	{
		colour = Colours::lightgreen;
	}

	g.fillAll(colour);

	// draw the cell bottom divider between rows
	g.setColour( Colours::black.withAlpha(0.5f));
	g.drawLine(0, height, width, height);

	return;
}


void dm::ClassIdWnd::paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (rowNumber < 0					or
		rowNumber >= (int)vinfo.size()	or
		columnId < 1					or
		columnId > 6					)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	String str = "?";
	if (columnId == 1) // original ID
	{
		str = String(vinfo[rowNumber].original_id);
	}
	else if (columnId == 2) // original name
	{
		str = vinfo[rowNumber].original_name;
	}
	else if (columnId == 3) // delete
	{
		// do nothing with this cell, we'll draw a checkbox instead
		str = "";
	}
	else if (columnId == 4) // merge into ID
	{
		const auto id = vinfo[rowNumber].merge_into_id;
		str = vinfo[id].modified_name;
	}
	else if (columnId == 5) // modified ID
	{
		if (vinfo[rowNumber].must_delete)
		{
			str = "";
		}
		else
		{
			str = String(vinfo[rowNumber].modified_id);
		}
	}
	else if (columnId == 6) // modified name
	{
		if (vinfo[rowNumber].must_delete)
		{
			str = "";
		}
		else
		{
			str = vinfo[rowNumber].modified_name;
		}
	}

	// draw the text and the right-hand-side dividing line between cells
	if (str.isNotEmpty())
	{
		g.setColour(Colours::black);
		Rectangle<int> r(0, 0, width, height);
		g.drawFittedText(str, r.reduced(2), Justification::centredLeft, 1 );
	}

	// draw the divider on the right side of the column
	g.setColour(Colours::black.withAlpha(0.5f));
	g.drawLine(width, 0, width, height);

	return;
}


void dm::ClassIdWnd::selectedRowsChanged(int rowNumber)
{
	if (rowNumber < 0					or
		rowNumber >= (int)vinfo.size()	)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	/// @todo

	table.repaint();

	return;
}


void dm::ClassIdWnd::cellClicked(int rowNumber, int columnId, const MouseEvent & event)
{
	std::cout << "cell clicked: row=" << rowNumber << " col=" << columnId << std::endl;

	if (rowNumber < 0					or
		rowNumber >= (int)vinfo.size()	or
		columnId != 4					)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	return;
}


Component * dm::ClassIdWnd::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component * existingComponentToUpdate)
{
	std::cout << "refresh: row=" << rowNumber << " col=" << columnId << " sel=" << isRowSelected << std::endl;

	if (rowNumber < 0					or
		rowNumber >= (int)vinfo.size()	)
	{
		// rows are 0-based, columns are 1-based
		return nullptr;
	}

	// column #3 must contain a togglebutton to delete the entire row
	if (columnId == 3)
	{
		existingComponentToUpdate = toggle_buttons[rowNumber];
	}

	// column #4 may contain a drop-down box to select where this ID will be merged
	if (columnId == 4)
	{
		// update the combobox to reflect the items that can actually be selected
		auto combobox = combo_boxes[rowNumber];
		combobox->clear();
		int next_item_idx = 1; // id=0 is reserved by JUCE
		combobox->addItem(" ", next_item_idx ++);
		combobox->addSeparator();
		for (size_t idx = 0; idx < vinfo.size(); idx ++)
		{
			combobox->addItem(vinfo[idx].modified_name, next_item_idx);

			if (idx == (size_t)rowNumber or
				vinfo[idx].must_delete or
				vinfo[idx].merge_into_id >= 0)
			{
				combobox->setItemEnabled(next_item_idx, false);
			}
			next_item_idx ++;
		}

		// should we pre-select some text in the combobox?
		const int merge_id = vinfo[rowNumber].merge_into_id;
		if (merge_id >= 0)
		{
			combobox->setText(vinfo[merge_id].modified_name);
		}

		existingComponentToUpdate = combobox;
	}

	return existingComponentToUpdate;
}


void dm::ClassIdWnd::rebuild_table()
{
	int changes = 0;
	int next_class_id = 0;

	for (size_t idx = 0; idx < vinfo.size(); idx ++)
	{
		auto & info = vinfo[idx];

		if (info.must_delete)
		{
			if (info.modified_id	!= -1 or
				info.modified_name	!= "")
			{
				info.modified_id = -1;
				info.modified_name = "";
				changes ++;
			}
		}
		else if (info.merge_into_id >= 0)
		{
			if (info.modified_id	!= vinfo[info.merge_into_id].modified_id or
				info.modified_name	!= vinfo[info.merge_into_id].modified_name)
			{
				info.modified_id	= vinfo[info.merge_into_id].modified_id;
				info.modified_name	= vinfo[info.merge_into_id].modified_name;
				changes ++;
			}
		}
		else
		{
			if (info.modified_id != next_class_id)
			{
				info.modified_id = next_class_id;
				changes ++;
			}

			if (info.modified_name.empty())
			{
				info.modified_name = info.original_name;
				changes ++;
			}

			next_class_id ++;
		}
	}

	// now go back through and assign all of the classes that were merged
	for (auto & info : vinfo)
	{
		const auto merge_id = info.merge_into_id;

		if (merge_id >= 0)
		{
			if (info.modified_id != vinfo[merge_id].modified_id)
			{
				info.modified_id = vinfo[merge_id].modified_id;
				changes ++;
			}

			if (info.modified_name != vinfo[merge_id].modified_name)
			{
				info.modified_name = vinfo[merge_id].modified_name;
				changes ++;
			}
		}
	}

	if (changes)
	{
//		table.updateContent();
//		table.repaint();
	}

	return;
}
