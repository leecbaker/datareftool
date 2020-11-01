#pragma once

#include "containers.h"
#include "widgets/text.h"

#include <chrono>
#include <functional>
#include <memory>

#include "search/search.h"

class RefRecord;

class SelectableListBase : public SingleAxisLayoutContainer {
protected:
    std::weak_ptr<Widget11> selected_element;
    std::weak_ptr<Widget11> tentative_selected_element;
    virtual void fireSelectionChange(std::weak_ptr<Widget11> & old_selection, std::weak_ptr<Widget11> & new_selection) = 0;
public:
    SelectableListBase() : SingleAxisLayoutContainer(SingleAxisLayoutContainer::LayoutAxis::VERTICAL) {}

    void draw(Rect visible_bounds) override;
    virtual std::shared_ptr<Widget11> mouseClick(Point point, XPLMMouseStatus status) override;

    virtual bool acceptsKeyboardFocus() const override { return true; }

    virtual void keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) override;


    virtual void clear() override {
        std::weak_ptr<Widget11> nothing;
        fireSelectionChange(selected_element, nothing);
        selected_element.reset();
        SingleAxisLayoutContainer::clear();
    }
};

template <class SelectableElement>
class SelectableList : public SelectableListBase {
    std::function<void(SelectableElement *, SelectableElement *)> selection_change_handler = nullptr;

    virtual void fireSelectionChange(std::weak_ptr<Widget11> & old_selection, std::weak_ptr<Widget11> & new_selection) override {
        if(nullptr == selection_change_handler) {
            return;
        }

        std::shared_ptr<Widget11> old_selection_widget_ptr = old_selection.lock();
        std::shared_ptr<Widget11> new_selection_widget_ptr = new_selection.lock();

        std::shared_ptr<SelectableElement> old_selection_ptr = std::dynamic_pointer_cast<SelectableElement>(old_selection_widget_ptr);
        std::shared_ptr<SelectableElement> new_selection_ptr = std::dynamic_pointer_cast<SelectableElement>(new_selection_widget_ptr);

        selection_change_handler(old_selection_ptr.get(), new_selection_ptr.get());
    }
public:
    void setSelectionChangeAction(std::function<void(SelectableElement*, SelectableElement *)> new_selection_change_handler) {
        selection_change_handler = new_selection_change_handler;
    }

    std::shared_ptr<SelectableElement> getSelection() const {
        return std::dynamic_pointer_cast<SelectableElement>(selected_element.lock());
    }
};

class ResultLine : public Widget11Text {
    RefRecord * record = nullptr;
    const std::chrono::system_clock::time_point * last_update_timestamp;
public:
    ResultLine(const std::chrono::system_clock::time_point * last_update_timestamp) : last_update_timestamp(last_update_timestamp) {}
    void setRecord(RefRecord *);
    RefRecord * getRecord() const { return record; }
    virtual void draw(Rect bounds) override;
};

class ResultsList : public SelectableList<ResultLine> {
    std::shared_ptr<SearchResults> & results;
public:
    ResultsList(std::shared_ptr<SearchResults> & results) : results(results) {}
    void update();
    virtual void draw(Rect draw_bounds) override;
};
