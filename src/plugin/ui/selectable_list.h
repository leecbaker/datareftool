#pragma once

#include "containers.h"
#include "widgets/text.h"

#include <chrono>
#include <functional>
#include <memory>

#include "search/search.h"

class RefRecord;
class SearchWindow;

class SelectableListBase : public SingleAxisLayoutContainer {
protected:
    SearchWindow * search_window;
    std::weak_ptr<LayoutObject> selected_element;
    virtual void fireSelectionChange(std::weak_ptr<LayoutObject> old_selection, std::weak_ptr<LayoutObject> new_selection) = 0;
public:
    SelectableListBase(SearchWindow * search_window) : SingleAxisLayoutContainer(SingleAxisLayoutContainer::LayoutAxis::VERTICAL), search_window(search_window) {}

    void draw(Rect visible_bounds) override;
    virtual std::shared_ptr<Widget11> mouseClick(Point point, XPLMMouseStatus status) override;

    virtual void clear() override {
        std::weak_ptr<LayoutObject> nothing;
        fireSelectionChange(selected_element, nothing);
        selected_element.reset();
        SingleAxisLayoutContainer::clear();
    }

    void deselect();
    void selectNext();
    void selectPrevious();
    void activateSelection();

    const SearchWindow * getSearchWindow() const { return search_window; }
    SearchWindow * getSearchWindow() { return search_window; }

    virtual bool keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) override;

    virtual bool acceptsKeyboardFocus() const override { return true; }
        // for keyboard focus purposes, this behaves as a single object
        virtual bool nextKeyboardFocus() override {
        if(hasKeyboardFocus()) {
            removeKeyboardFocus();
            return false;
        }

        std::shared_ptr<Widget11> selected_element_widget = std::dynamic_pointer_cast<Widget11, LayoutObject>(selected_element.lock());

        if(acceptsKeyboardFocus() && selected_element_widget) {
            selected_element_widget->giveKeyboardFocus();
            return true;
        }

        return false;
    }

    virtual bool previousKeyboardFocus() override {
        if(hasKeyboardFocus()) {
            removeKeyboardFocus();
            return false;
        }

        std::shared_ptr<Widget11> selected_element_widget = std::dynamic_pointer_cast<Widget11, LayoutObject>(selected_element.lock());

        if(acceptsKeyboardFocus() && selected_element_widget) {
            selected_element_widget->giveKeyboardFocus();
            return true;
        }

        return false;
    }
};

template <class SelectableElement>
class SelectableList : public SelectableListBase {
    std::function<void(SelectableElement *, SelectableElement *)> selection_change_handler = nullptr;

    virtual void fireSelectionChange(std::weak_ptr<LayoutObject> old_selection, std::weak_ptr<LayoutObject> new_selection) override {
        if(nullptr == selection_change_handler) {
            return;
        }

        std::shared_ptr<LayoutObject> old_selection_widget_ptr = old_selection.lock();
        std::shared_ptr<LayoutObject> new_selection_widget_ptr = new_selection.lock();

        std::shared_ptr<SelectableElement> old_selection_ptr = std::dynamic_pointer_cast<SelectableElement>(old_selection_widget_ptr);
        std::shared_ptr<SelectableElement> new_selection_ptr = std::dynamic_pointer_cast<SelectableElement>(new_selection_widget_ptr);

        selection_change_handler(old_selection_ptr.get(), new_selection_ptr.get());
    }
public:
    SelectableList(SearchWindow * search_window) : SelectableListBase(search_window) {}
    void setSelectionChangeAction(std::function<void(SelectableElement*, SelectableElement *)> new_selection_change_handler) {
        selection_change_handler = new_selection_change_handler;
    }

    std::shared_ptr<SelectableElement> getSelection() const {
        return std::dynamic_pointer_cast<SelectableElement>(selected_element.lock());
    }
};

class ResultsList;

class ResultLine : public Widget11Text {
    RefRecord * record = nullptr;
    const std::chrono::system_clock::time_point * last_update_timestamp;
public:
    ResultLine(const std::chrono::system_clock::time_point * last_update_timestamp) : last_update_timestamp(last_update_timestamp) {}
    void setRecord(RefRecord *);
    RefRecord * getRecord() const { return record; }
    virtual void draw(Rect bounds) override;

    virtual bool acceptsKeyboardFocus() const override { return true; }
    virtual bool keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) override;
};

class ResultsList : public SelectableList<ResultLine> {
    std::shared_ptr<SearchResults> & results;
public:
    ResultsList(SearchWindow * search_window, std::shared_ptr<SearchResults> & results) : SelectableList<ResultLine>(search_window), results(results) {}
    void update();
    virtual void draw(Rect draw_bounds) override;
};
