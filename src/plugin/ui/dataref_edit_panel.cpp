#include "dataref_edit_panel.h"

#include "widgets/button.h"
#include "widgets/spacer.h"

#include "search/dataref.h"

#include "../drt_plugin.h"


bool checkValueOK(DataRefRecord * drr, const std::string & new_value) {
    if(drr->isDouble()) {
        try {
            std::stold(new_value);
        } catch(std::exception &) {
            return false;
        }
    } else if(drr->isFloat() || drr->isFloatArray()) {
        try {
            std::stof(new_value);
        } catch(std::exception &) {
            return false;
        }
    } else if(drr->isInt() || drr->isIntArray()) {
        try {
            std::stoi(new_value);
        } catch(std::exception &) {
            return false;
        }
    } else if(drr->isData()) {
    }

    return true;
}

void setValue(DataRefRecord * drr, const std::string & new_value, std::optional<int> index) {
    if(drr->isDouble()) {
        assert(!index);
        try {
            double d = std::stold(new_value);
            drr->setDouble(d);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isFloat()) {
        assert(!index);
        try {
            float f = std::stof(new_value);
            drr->setFloat(f);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isInt()) {
        assert(!index);
        try {
            int i = std::stoi(new_value);
            drr->setInt(i);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isFloatArray()) {
        assert(index);
        try {
            float f = std::stof(new_value);
            drr->setFloatArrayElement(f, *index);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isIntArray()) {
        assert(index);
        try {
            int i = std::stoi(new_value);
            drr->setIntArrayElement(i, *index);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isData()) {
        assert(!index);
        drr->setData(new_value);
    }
}

void DatarefEditField::draw(Rect visible_bounds) {
    if(false == hasKeyboardFocus() && displayed_values_updated != drr->getLastUpdateTime()) {
        setContents(drr->getArrayElementEditString(element_index));
        displayed_values_updated = drr->getLastUpdateTime();
    }

    Widget11TextField::draw(visible_bounds);
}

DatarefEditField::DatarefEditField(DataRefRecord * drr, int element_index)
: drr(drr)
, element_index(element_index)
{
    setPlaceholder("New value");

    setContents(drr->getArrayElementEditString(element_index));

    setTypeAction([this, drr]() {
        if(checkValueOK(drr, getContents())) {
        } else {
        }
    });

    std::optional<int> index_in_array_if_array;
    if(drr->isArray()) {
        index_in_array_if_array = element_index;
    }

    std::function<void()> set_value_edit_done = [element_index, this, drr, index_in_array_if_array]() {
        setValue(drr, getContents(), index_in_array_if_array);
        
        // This might seem like a great time to update the displayed value, but
        // at this time, we haven't actually read the DR's value back yet to know if it
        // changed. We can trigger a read by updating all DRs now, and then update the
        //displayed value.

        plugin->getRefs().update();
        setContents(drr->getArrayElementEditString(element_index));
        selectAll();
    };

    setSubmitAction(set_value_edit_done);
    setEnabled(drr->writable());
}

DatarefEditPanel::DatarefEditPanel(DataRefRecord * drr, int element_index)
: SingleAxisLayoutContainer(SingleAxisLayoutContainer::LayoutAxis::HORIZONTAL)
, drr(drr)
, element_index(element_index)
{
    std::shared_ptr<Widget11Text> index_label;
    if(drr->isArray()) {
        index_label = std::make_shared<Widget11Text>();
        index_label->setText(std::to_string(element_index));
        index_label->setMinimumSize({30, 0});
    }

    edit_field = std::make_shared<Widget11TextField>();
    edit_field->setPlaceholder("New value");

    edit_field->setContents(drr->getArrayElementEditString(element_index));

    std::shared_ptr<Widget11Spacer> edit_spacer = std::make_shared<Widget11Spacer>(Size{20, 1});

    std::shared_ptr<Widget11Button> set_button;
    set_button = std::make_shared<Widget11Button>();
    set_button->setLabel("Set");
    set_button->setMinimumSize({40, 0});

    edit_field->setTypeAction([this, set_button, drr]() {
        if(checkValueOK(drr, edit_field->getContents())) {
            set_button->setEnabled(true);
        } else {
            set_button->setEnabled(false);
        }
    });

    std::optional<int> index_in_array_if_array;
    if(drr->isArray()) {
        index_in_array_if_array = element_index;
    }

    std::function<void()> set_value_edit_done = [element_index, this, drr, index_in_array_if_array]() {
        setValue(drr, edit_field->getContents(), index_in_array_if_array);

        // This might seem like a great time to update the displayed value, but
        // at this time, we haven't actually read the DR's value back yet to know if it
        // changed. We can trigger a read by updating all DRs now, and then update the
        //displayed value.

        plugin->getRefs().update();
        edit_field->setContents(drr->getArrayElementEditString(element_index));
        edit_field->selectAll();
    };

    edit_field->setSubmitAction(set_value_edit_done);
    set_button->setAction(set_value_edit_done);

    edit_field->setEnabled(drr->writable());

    if(index_label) {
        this->add(index_label, false, false);
    }
    this->add(edit_field, true, true);
    this->add(edit_spacer, false, false);
    if(drr->writable()) {
        this->add(set_button, false, false);
    }
}

void DatarefEditPanel::update() {
    if(false == edit_field->hasKeyboardFocus()) {
        edit_field->setContents(drr->getArrayElementEditString(element_index));
    }
}