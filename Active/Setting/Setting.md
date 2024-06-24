
# Setting

## Contents
1. [Purpose](#purpose)
2. [Overview](#over)
3. [Application](#application)

## Purpose <a name="purpose"></a>

For the purpose of this module, a "setting" generally means "some data item", but more particularly associated with parameters or preferences for inter-process communication. The Settings module provides essential infrastructure for modules that exchange data, e.g. `Event` and `Serialise`. The `Event` module in particular is designed to facilitate cooperation between discrete components without direct coupling or dependencies (separation of concerns). In that context, the `Setting` module provides a mechanism for transporting data between components/processes.

## Overview <a name="over"></a>

Consider a simple graphics application as an example for the role of Settings in the context of Events:
- The app displays 2D lines that can be selected by the user
- A toolbar displays the length and angle of the selected line and allows the user to edit them (updating the line accordingly)
- The length and angle are displayed with the user's preferred units, e.g. metres for length and decimal degrees for angle (but they could include feet/inches, radians, surveyor bearings etc)
- A standard UI component is used to display the length/angle, which basically displays text and supports user editing of the text

Development requirements will include:
- Formatting text according to a specific unit convention, e.g. 2.5m/2500mm/8' 2.425" or 32.5°/0.5672rad/N 57°30'00" E
- Validating user input (according to the unit type)
- Converting user input to app standard e.g. lengths stored in metres and entries in other units converted on the fly
- Responding to changes in the users preferred units
- Applying valid user input to the data model

Events and Settings can play a significant role in the solution:
- A `ValueSetting` wraps a value with the logic to convert it to and from a string - an ideal interface for a text field. Injected as a variable in the text field causes it to take on the characteristics of the value type, e.g. a `LengthValue` makes the field behave as a measurement of length. It operations include:
	- Validating text entered by the user
	- Performing unit conversions
	- Providing content when the field is refreshed
- A method for establishing the current preferences can also be embedded in a `ValueSetting`, enabling it to respond to changes with no explicit coupling to application or preferences logic.
- When the user enters new data and the embedded setting has validated it, the field can emit an `Event` with the `ValueSetting` attached
- Any tool intended to apply user input to a shape can subscribe to a suitable `Event`. On receipt, it can extract the new value from the attached `ValueSetting` and apply it to the model - this all happens with no coupling between the UI and the tool.

## Application <a name="application"></a>

The base `Setting` defines the interface for any interprocess data. At this level, it only concretely defines an identity, consisting of any combination of:
- Name: Can be anything, but the choice can help with readability/logging
- Guid: Optional, but useful where names may not be unique, e.g. particularly where they may have been defined by independent developers

The identity enables the recipient to unambiguously locate essential data. For example, a tool that subscribes to a 'user tap/click' might inspect an incoming event for a setting specifying the number of presses (single/double/triple-click):
```Cpp
	///Receive a subscribed event - return true if event request is closed by this tool
bool receive(const Event& event) {
		//This tool wants a setting for the number of taps, and reacts when there is a double-tap
	if (auto tapCount = event.findValue(tapCountID); (tapCount != nullptr) && !tapCount->empty() && (tapCount->int32Val() == 2)) {
		//Do whatever the tool is supposed to do here…
	}
	return false;
}
```
…where the setting identity might be defined like this:
```Cpp
const String tapCountID{"tapCount"};
```
…or in  situations where there is a risk of name clashes, it could be defined with a GUID:
```Cpp
const NameID tapCountID{"tapCount", String{"ad870214-1e56-450c-b74c-f8a6711d276b"}};
```
`Event` is derived from `SettingList`, so inherits functionality for attaching and finding settings. The author of an `Event` will insert whatever settings it supports before publishing, but with no knowledge of which subscribers might receive it or how/when the attached settings might be used. This leaves an open path for additional functionality to be built in response to events with no changes or reference to the code at the point of origin.

This principle is extended by abstracting data values. A scenario is described in the overview (above) where a text field is intended to display and support input for a length measurement, achieved by embedding an abstract `ValueSetting` variable.
> NB: The following example is a generic mock-up, i.e. not based on any specific UI framework
```Cpp
class TextField {
public:
	TextField(const ValueSetting& setting, NameID::Option changeID = std::nullopt) : m_setting{clone(setting)}, m_changeID{changeID} {}

		///Respond to a request to refresh the displayed content
	void onRefresh() {
		if (m_setting)
			setText(m_setting->stringVal());
	}

		///Respond to a change in the field text
	void onChange() {
		if (!m_setting || m_setting->empty())
			return;
			//Make a clone of the embedded value and assign the new text
		auto sampleSetting = clone(*(*m_setting)[0]);
		*sampleSetting = getText();
			//If the assignment is good, update the embedded setting - otherwise keep the existing value
		if (sampleSetting->isGood()) {
			(*m_setting)[0] = std::move(sampleSetting);
				//If an event ID is specified, publish the change (including the new value)
			if (m_changeID)
				app->publish(Event{*m_changeID, SettingList{*m_setting}});
		}
		setText(m_setting->stringVal());
	}
private:
	ValueSetting::Unique m_setting;
	NameID::Option m_changeID;
};
```
The constructor takes a `ValueSetting` (which determines its behaviour) and optionally a `NameID` to be used if changes should be published.
The functions are mocked up for illustrative purposes, but reflect common functionality in UI frameworks:
- `onRefresh`: Called when the field should set the latest text to be displayed in the field (with `setText`). It only has to ask the embedded setting for a text value. The response will reflect whatever behaviour is built into the embedded ValueSetting, e.g. for a value of 1.5 `LengthValue` might return "1500mm" whereas `AngleValue` might return "86.94°" (depending on unit preferences). The critical point is that the `TextField` is not coupled to any of it.
- `onChange`: Called when the user has entered new text (retrieved with `getText`). The new text is simply handed over to a clone of the embedded value to test its validity (by calling `Value::isGood()`):
	- When valid, it updates the embedded value. If an event ID is specified, it also publishes the change (with the new value attached in a `SettingList`).
	- Otherwise it retains the existing value
	- The displayed text is updated in either case. The new text will be formatted according to the `ValueSetting` criteria, e.g. if it's a `LengthValue`, the unit are set to millimetres and the user enters 2' 3", the new text will become "685.8mm".

If the identity of an event signalling a length change is `changeLengthID`, e.g.:
```Cpp
const NameID changeLengthID{"changeLength", String{"67d60a81-52e6-47ad-8d10-1ac447bcb0f5"}};;
```
…and the identity of a length value is `lengthID`, e.g.:
```Cpp
const NameID lengthID{"newLength", String{"ac580a1a-a904-4768-bc08-569d9ef1af8c"}};;
```
…then the TextField could be created like this:
```Cpp
TextField entryField{{LengthValue{currentLength}, lengthID}, changeLengthID};
```
A tool that wanted notification of length changes could be declared like this:
```Cpp
class ShapeLengthTool : public Subscriber {
public:
		///Get a list of required subscriptions
	Subscription subscription() const override { return { changeLengthID }; }

		///Receive a subscribed event - return true if event request is handled by this tool
	bool receive(const active::event::Event& event) override {
		if (auto length = event.findValue(lengthID); (length != nullptr) && !length->empty()) {
				//Do whatever the tool is supposed to do here…
			selectedShape->setLength(*length);
			return true;
		}
		return false;
	}
};
```
The tool simply includes `changeLengthID` in its subscription list and searches any incoming `Event` for a setting with the ID `lengthID`. Although this value was updated from a text source, the value presents itself as the correct type (and in this case in the correct units) to the tool.

This abstraction of value type allows each component to use it as if it was native type it expected without having to understand where it came from or what type it might be internally, decoupling them from the knowledge of peforming conversions (and typically preventing subtle duplication of that code).