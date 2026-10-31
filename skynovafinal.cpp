// SKYNOVA - Smart Airline Reservation System
// C++11 / Win32 GUI / OOP demonstration project
// All flights, prices, and predictions are simulated for academic demonstration.

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>

#pragma comment(lib, "comctl32.lib")

using namespace std;

#define ID_FROM       101
#define ID_TO         102
#define ID_DATE       103
#define ID_CLASS      104
#define ID_MEAL       105
#define ID_SEAT_PREF  106
#define ID_PASSENGER  107
#define ID_MIN_FARE   108
#define ID_MAX_FARE   109
#define ID_SEARCH     301
#define ID_BOOK       302
#define ID_STATUS     303
#define ID_CANCEL     304
#define ID_ADVISOR    305
#define ID_LIST       310
#define ID_TICKET_ID  311
#define ID_TICKET_OUT 312

const int TOTAL_SEATS = 48; // 8 rows x 6 seats (A-F)

class City {
public:
    string name;
    City(const string& n = "") : name(n) {}
};

class Flight {
public:
    string airline, number, from, to, date, departure, arrival, travelClass, meal;
    int fare, seats;
    Flight() : fare(0), seats(TOTAL_SEATS) {}
};

class Booking {
public:
    string id, passenger, status, payment, seat, seatPreference;
    Flight flight;
    Booking() : status("CONFIRMED"), payment("PAYMENT SUCCESSFUL") {}
};

// Abstract base class: defines the common interface for fare calculation.
class FarePolicy {
public:
    virtual int calculateFare(int days, const string& airline,
                              const string& travelClass, const string& meal) const = 0;
    virtual ~FarePolicy() {}
};

// Inheritance: DynamicFarePolicy derives from FarePolicy.
// It keeps the same fare calculation rules used by the original project.
class DynamicFarePolicy : public FarePolicy {
public:
    int calculateFare(int days, const string& airline,
                      const string& travelClass, const string& meal) const override {
        int fare = 3500;
        if (airline == "Air India") fare += 700;
        else if (airline == "Akasa Air") fare -= 250;
        if (travelClass == "Business") fare += 6500;
        else if (travelClass == "Premium Economy") fare += 1800;
        if (meal == "Veg Meal" || meal == "Non-Veg Meal") fare += 250;
        if (days >= 30) fare = fare * 85 / 100;
        else if (days >= 14) fare = fare * 92 / 100;
        else if (days <= 2) fare = fare * 125 / 100;
        return fare;
    }
};

class AirlineSystem {
private:
    vector<Booking> bookings;
    int nextId;
    DynamicFarePolicy dynamicFarePolicy;
public:
    vector<City> cities;
    vector<Flight> results;

    AirlineSystem() : nextId(1001) {
        const char* names[] = {"Amritsar", "Ahmedabad", "Bengaluru", "Bhopal", "Bhubaneswar", "Chandigarh", "Chennai", "Coimbatore", "Dehradun", "Delhi", "Goa", "Guwahati", "Hyderabad", "Indore", "Jaipur", "Jammu", "Kochi", "Kolkata", "Lucknow", "Madurai", "Mangaluru", "Mumbai", "Mysuru", "Nagpur", "Patna", "Pune", "Raipur", "Ranchi", "Srinagar", "Surat", "Thiruvananthapuram", "Tirupati", "Udaipur", "Vadodara", "Varanasi", "Vijayawada", "Visakhapatnam"};
        for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) cities.push_back(City(names[i]));
    }

    int bookedCount(const Flight& f) const {
        int count = 0;
        for (size_t i = 0; i < bookings.size(); ++i) {
            const Booking& b = bookings[i];
            if (b.status == "CONFIRMED" && b.flight.number == f.number && b.flight.from == f.from && b.flight.to == f.to && b.flight.date == f.date) ++count;
        }
        return count;
    }

    bool seatTaken(const Flight& f, const string& seat) const {
        for (size_t i = 0; i < bookings.size(); ++i) {
            const Booking& b = bookings[i];
            if (b.status == "CONFIRMED" && b.flight.number == f.number && b.flight.from == f.from && b.flight.to == f.to && b.flight.date == f.date && b.seat == seat) return true;
        }
        return false;
    }

    int remainingSeats(const Flight& f) const { return TOTAL_SEATS - bookedCount(f); }

    int baseFare(int days, const string& airline, const string& travelClass, const string& meal) const {
        // Runtime polymorphism: call the overridden function through a base-class pointer.
        const FarePolicy* policy = &dynamicFarePolicy;
        return policy->calculateFare(days, airline, travelClass, meal);
    }

    int daysUntil(const string& dateText) const {
        int y = 0, m = 0, d = 0;
        if (sscanf(dateText.c_str(), "%d-%d-%d", &y, &m, &d) != 3) return 7;
        tm target = {};
        target.tm_year = y - 1900; target.tm_mon = m - 1; target.tm_mday = d; target.tm_hour = 12;
        time_t targetTime = mktime(&target);
        time_t now = time(NULL);
        tm today = *localtime(&now); today.tm_hour = 12; today.tm_min = 0; today.tm_sec = 0;
        double seconds = difftime(targetTime, mktime(&today));
        return (int)(seconds / 86400.0);
    }

    void search(const string& from, const string& to, const string& date, const string& travelClass, const string& meal, int minFare, int maxFare) {
        results.clear();
        if (from.empty() || to.empty() || from == to) return;
        // Nine illustrative flight options with different airlines and timings.
        const char* airlines[] = {"IndiGo", "Air India", "Akasa Air", "IndiGo", "Air India", "Akasa Air", "IndiGo", "Air India", "Akasa Air"};
        const char* numbers[] = {"6E201", "AI431", "QP612", "6E315", "AI605", "QP728", "6E442", "AI819", "QP936"};
        const char* departures[] = {"06:15", "08:40", "10:20", "12:30", "14:10", "16:25", "18:45", "20:15", "22:05"};
        const char* arrivals[] = {"07:25", "09:50", "11:30", "13:40", "15:20", "17:35", "19:55", "21:25", "23:15"};
        int days = daysUntil(date);
        for (int i = 0; i < 9; ++i) {
            Flight f;
            f.airline = airlines[i]; f.number = numbers[i]; f.from = from; f.to = to; f.date = date;
            f.departure = departures[i]; f.arrival = arrivals[i]; f.travelClass = travelClass; f.meal = meal;
            f.fare = baseFare(days, f.airline, travelClass, meal);
            int occupied = bookedCount(f);
            double occupancy = (double)occupied / TOTAL_SEATS;
            if (occupancy >= 0.85) f.fare = f.fare * 120 / 100;
            else if (occupancy >= 0.60) f.fare = f.fare * 110 / 100;
            f.seats = TOTAL_SEATS - occupied;
            if (f.fare >= minFare && f.fare <= maxFare) results.push_back(f);
        }
    }

    string nextSeat(const Flight& f, const string& preference) const {
        const char letters[] = {'A', 'B', 'C', 'D', 'E', 'F'};
        for (int row = 1; row <= 8; ++row) {
            for (int col = 0; col < 6; ++col) {
                char letter = letters[col];
                bool window = (letter == 'A' || letter == 'F');
                if (preference == "Window Seat" && !window) continue;
                if (preference == "Normal Seat" && window) continue;
                stringstream ss; ss << row << letter;
                if (!seatTaken(f, ss.str())) return ss.str();
            }
        }
        return "";
    }

    bool createBooking(const Flight& f, const string& passenger, const string& preference, Booking& created) {
        if (remainingSeats(f) <= 0) return false;
        string seat = nextSeat(f, preference);
        if (seat.empty()) return false;
        created.id = "SKY" + to_string(nextId++);
        created.passenger = passenger; created.flight = f; created.seat = seat;
        created.seatPreference = preference; created.status = "CONFIRMED";
        bookings.push_back(created);
        return true;
    }

    Booking* findBooking(const string& id) {
        for (size_t i = 0; i < bookings.size(); ++i) if (bookings[i].id == id) return &bookings[i];
        return NULL;
    }
};

AirlineSystem systemData;
HWND hMain = NULL, hFrom = NULL, hTo = NULL, hDate = NULL, hClass = NULL, hMeal = NULL, hSeatPref = NULL;
HWND hPassenger = NULL, hMinFare = NULL, hMaxFare = NULL, hList = NULL, hTicketID = NULL, hTicketOutput = NULL, hMessage = NULL;
HFONT hFont = NULL, hBoldFont = NULL;
HBRUSH hBackground = NULL, hWhite = NULL;

string getText(HWND h) { char buffer[512] = {0}; GetWindowTextA(h, buffer, sizeof(buffer)); return string(buffer); }
void setText(HWND h, const string& s) { SetWindowTextA(h, s.c_str()); }
int getNumber(HWND h, int fallback) { string s = getText(h); if (s.empty()) return fallback; return atoi(s.c_str()); }

void addLabel(HWND parent, const char* text, int x, int y, int w, int h) {
    HWND c = CreateWindowA("STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, h, parent, NULL, NULL, NULL);
    SendMessage(c, WM_SETFONT, (WPARAM)hFont, TRUE);
}
HWND addEdit(HWND parent, int id, const char* value, int x, int y, int w, int h, DWORD extra = 0) {
    HWND c = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", value, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | extra, x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
    SendMessage(c, WM_SETFONT, (WPARAM)hFont, TRUE); return c;
}
HWND addButton(HWND parent, int id, const char* value, int x, int y, int w, int h) {
    HWND c = CreateWindowA("BUTTON", value, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
    SendMessage(c, WM_SETFONT, (WPARAM)hBoldFont, TRUE); return c;
}
HWND addCombo(HWND parent, int id, const vector<string>& values, int x, int y, int w, int h) {
    HWND c = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
    for (size_t i = 0; i < values.size(); ++i) SendMessageA(c, CB_ADDSTRING, 0, (LPARAM)values[i].c_str());
    SendMessage(c, CB_SETCURSEL, 0, 0); SendMessage(c, WM_SETFONT, (WPARAM)hFont, TRUE); return c;
}
string comboText(HWND h) { int index = (int)SendMessage(h, CB_GETCURSEL, 0, 0); if (index < 0) return ""; char b[256] = {0}; SendMessageA(h, CB_GETLBTEXT, index, (LPARAM)b); return b; }

void setMessage(const string& message) { if (hMessage) setText(hMessage, message); }
int columnCount = 0;
void listAddColumn(const char* title, int width) {
    LVCOLUMNA c = {};
    c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    c.pszText = (LPSTR)title;
    c.cx = width;
    c.iSubItem = columnCount;
    ListView_InsertColumn(hList, columnCount, &c);
    columnCount++;
}
void addCell(int row, int col, const string& value) { if (col == 0) { LVITEMA item = {}; item.mask = LVIF_TEXT; item.iItem = row; item.pszText = (LPSTR)value.c_str(); ListView_InsertItem(hList, &item); } else ListView_SetItemText(hList, row, col, (LPSTR)value.c_str()); }
int selectedRow() { return ListView_GetNextItem(hList, -1, LVNI_SELECTED); }
string money(int amount) { stringstream ss; ss << "Rs. " << amount; return ss.str(); }

void performSearch() {
    string from = comboText(hFrom), to = comboText(hTo), date = getText(hDate);
    if (from.empty() || to.empty() || from == to) { MessageBoxA(hMain, "Please choose two different cities.", "SKYNOVA", MB_ICONWARNING); return; }
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') { MessageBoxA(hMain, "Enter the date in YYYY-MM-DD format, for example 2026-11-15.", "Date format", MB_ICONWARNING); return; }
    int minFare = getNumber(hMinFare, 0), maxFare = getNumber(hMaxFare, 1000000);
    systemData.search(from, to, date, comboText(hClass), comboText(hMeal), minFare, maxFare);
    ListView_DeleteAllItems(hList);
    // Clear any old selection before showing the new search results.
    ListView_SetItemState(hList, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
    for (size_t i = 0; i < systemData.results.size(); ++i) {
        const Flight& f = systemData.results[i]; int r = (int)i;
        addCell(r, 0, f.number); addCell(r, 1, f.airline); addCell(r, 2, f.from + " - " + f.to);
        addCell(r, 3, f.date); addCell(r, 4, f.departure); addCell(r, 5, f.arrival);
        addCell(r, 6, f.travelClass); addCell(r, 7, f.meal); addCell(r, 8, money(f.fare));
        addCell(r, 9, to_string(systemData.remainingSeats(f))); addCell(r, 10, f.fare <= 5000 ? "Within budget" : "Compare fare");
    }
    setMessage(to_string(systemData.results.size()) + " simulated flight option(s) found. Seat inventory: 48 seats per flight.");
}

string makeTicket(const Booking& b) {
    stringstream ss;
    ss << "================================================\r\n";
    ss << "             SKYNOVA  |  E-TICKET\r\n";
    ss << "================================================\r\n";
    ss << "Booking ID : " << b.id << "\r\n";
    ss << "Passenger  : " << b.passenger << "\r\n";
    ss << "Status     : " << b.status << "\r\n";
    ss << "------------------------------------------------\r\n";
    ss << "Flight     : " << b.flight.number << " (" << b.flight.airline << ")\r\n";
    ss << "Route      : " << b.flight.from << "  ->  " << b.flight.to << "\r\n";
    ss << "Date       : " << b.flight.date << "\r\n";
    ss << "Departure  : " << b.flight.departure << "     Arrival: " << b.flight.arrival << "\r\n";
    ss << "Class      : " << b.flight.travelClass << "\r\n";
    ss << "Meal       : " << b.flight.meal << "\r\n";
    ss << "Seat       : " << b.seat << " (" << b.seatPreference << ")\r\n";
    ss << "Fare       : " << money(b.flight.fare) << "\r\n";
    ss << "Payment    : " << b.payment << "\r\n";
    ss << "================================================\r\n";
    ss << "Academic Project|Simulated Flight Data|No Real Bookings or Payments\r\n";
    return ss.str();
}

void bookSelected() {
    int row = selectedRow();
    if (row < 0 || row >= (int)systemData.results.size()) { MessageBoxA(hMain, "Select a flight from the results first.", "Book flight", MB_ICONINFORMATION); return; }
    string passenger = getText(hPassenger);
    if (passenger.empty()) { MessageBoxA(hMain, "Enter the passenger name first.", "Passenger required", MB_ICONWARNING); return; }
    Flight f = systemData.results[row];
    string pref = comboText(hSeatPref);
    Booking b;
    if (!systemData.createBooking(f, passenger, pref, b)) { MessageBoxA(hMain, "No seat is available for this preference. Try the other seat type or another flight.", "No seat available", MB_ICONWARNING); return; }
    setText(hTicketID, b.id); setText(hTicketOutput, makeTicket(b));
    setMessage("Booking confirmed. Assigned seat " + b.seat + ". Booking ID: " + b.id);
    MessageBoxA(hMain, ("Booking confirmed!\nBooking ID: " + b.id + "\nSeat: " + b.seat + "\n\nYour ticket summary is displayed below.").c_str(), "SKYNOVA e-ticket", MB_ICONINFORMATION);
    performSearch();
}

void checkStatus() {
    string id = getText(hTicketID);
    if (id.empty()) { MessageBoxA(hMain, "Enter a booking ID, for example SKY1001.", "Booking status", MB_ICONINFORMATION); return; }
    Booking* b = systemData.findBooking(id);
    if (!b) { setText(hTicketOutput, "No booking found for ID: " + id); setMessage("Booking ID not found."); return; }
    setText(hTicketOutput, makeTicket(*b)); setMessage("Status for " + id + ": " + b->status);
}

void cancelBooking() {
    string id = getText(hTicketID);
    Booking* b = systemData.findBooking(id);
    if (!b) { MessageBoxA(hMain, "Enter a valid booking ID first.", "Cancel booking", MB_ICONWARNING); return; }
    if (b->status == "CANCELLED") { MessageBoxA(hMain, "This booking is already cancelled.", "Cancel booking", MB_ICONINFORMATION); return; }
    if (MessageBoxA(hMain, "Cancel this demo booking? The seat will become available again.", "Confirm cancellation", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    b->status = "CANCELLED"; b->payment = "DEMO CANCELLATION - NO REAL REFUND";
    setText(hTicketOutput, makeTicket(*b)); setMessage("Booking " + id + " cancelled. Seat " + b->seat + " is available again.");
    performSearch();
}

void buyWaitAdvisor() {
    int row = selectedRow();
    if (row < 0 || row >= (int)systemData.results.size()) { MessageBoxA(hMain, "Select a flight first to get the Buy Now or Wait advice.", "Fare advisor", MB_ICONINFORMATION); return; }
    Flight f = systemData.results[row];
    int left = systemData.remainingSeats(f); int occupied = TOTAL_SEATS - left;
    int days = systemData.daysUntil(f.date);
    int predicted = f.fare;
    string reason;
    if (left <= 8) { predicted = f.fare * 120 / 100; reason = "Few seats remain; the illustrative model expects a higher fare."; }
    else if (days <= 3) { predicted = f.fare * 115 / 100; reason = "Departure is close; the illustrative model expects a higher fare."; }
    else if (days >= 21 && occupied < 24) { predicted = f.fare * 92 / 100; reason = "There is time before departure and many seats remain; waiting may be reasonable."; }
    else { predicted = f.fare * 105 / 100; reason = "The model expects a small increase as departure approaches."; }
    string decision = (left <= 8 || days <= 3) ? "RECOMMENDATION: BUY NOW" : (days >= 21 && occupied < 24 ? "RECOMMENDATION: YOU MAY WAIT" : "RECOMMENDATION: COMPARE, THEN DECIDE");
    stringstream ss;
    ss << "SKYNOVA - BUY NOW OR WAIT ADVISOR\n\n";
    ss << "Flight: " << f.number << " (" << f.airline << ")\n";
    ss << "Current simulated fare: " << money(f.fare) << "\n";
    ss << "Illustrative possible next fare: " << money(predicted) << "\n";
    ss << "Seats remaining: " << left << " / " << TOTAL_SEATS << "\n";
    ss << "Days until departure: " << days << "\n\n" << decision << "\n" << reason << "\n\n";
    ss << "Important: This is a classroom heuristic, not live airline pricing or a guaranteed forecast.";
    MessageBoxA(hMain, ss.str().c_str(), "Buy Now or Wait Advisor", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        hMain = hwnd;
        addLabel(hwnd, "SKYNOVA - SMART AIRLINE RESERVATION", 22, 15, 700, 30);
        addLabel(hwnd, "From", 22, 58, 55, 20); addLabel(hwnd, "To", 220, 58, 35, 20); addLabel(hwnd, "Date (YYYY-MM-DD)", 420, 58, 140, 20);
        vector<string> cityNames; for (size_t i = 0; i < systemData.cities.size(); ++i) cityNames.push_back(systemData.cities[i].name);
        hFrom = addCombo(hwnd, ID_FROM, cityNames, 22, 80, 175, 250); hTo = addCombo(hwnd, ID_TO, cityNames, 220, 80, 175, 250); SendMessage(hTo, CB_SETCURSEL, 9, 0);
        hDate = addEdit(hwnd, ID_DATE, "2026-11-15", 420, 80, 145, 25);
        addLabel(hwnd, "Travel class", 590, 58, 90, 20); addLabel(hwnd, "Meal", 715, 58, 45, 20);
        vector<string> classes; classes.push_back("Economy"); classes.push_back("Premium Economy"); classes.push_back("Business");
        vector<string> meals; meals.push_back("No Meal"); meals.push_back("Veg Meal"); meals.push_back("Non-Veg Meal");
        hClass = addCombo(hwnd, ID_CLASS, classes, 590, 80, 120, 200); hMeal = addCombo(hwnd, ID_MEAL, meals, 715, 80, 135, 200);
        addLabel(hwnd, "Min fare", 22, 120, 65, 20); addLabel(hwnd, "Max fare", 130, 120, 65, 20);
        hMinFare = addEdit(hwnd, ID_MIN_FARE, "0", 22, 142, 90, 25); hMaxFare = addEdit(hwnd, ID_MAX_FARE, "20000", 130, 142, 90, 25);
        addLabel(hwnd, "Passenger name", 245, 120, 115, 20); hPassenger = addEdit(hwnd, ID_PASSENGER, "", 245, 142, 170, 25);
        addLabel(hwnd, "Seat preference", 440, 120, 110, 20); vector<string> seats; seats.push_back("Window Seat"); seats.push_back("Normal Seat");
        hSeatPref = addCombo(hwnd, ID_SEAT_PREF, seats, 440, 142, 135, 200);
        addButton(hwnd, ID_SEARCH, "Search Flights", 595, 137, 115, 34); addButton(hwnd, ID_BOOK, "Book Selected", 720, 137, 130, 34);
        addLabel(hwnd, "Available simulated flights (48 seats per flight)", 22, 185, 400, 20);
        hList = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 22, 207, 828, 190, hwnd, (HMENU)(INT_PTR)ID_LIST, NULL, NULL);
       ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        columnCount = 0;
        listAddColumn("Flight", 65); listAddColumn("Airline", 80); listAddColumn("Route", 145); listAddColumn("Date", 85); listAddColumn("Depart", 60); listAddColumn("Arrive", 60); listAddColumn("Class", 95); listAddColumn("Meal", 90); listAddColumn("Fare", 85); listAddColumn("Seats", 45); listAddColumn("Budget", 100);
        addLabel(hwnd, "Booking ID", 22, 414, 75, 20); hTicketID = addEdit(hwnd, ID_TICKET_ID, "", 100, 410, 130, 26);
        addButton(hwnd, ID_STATUS, "Check Status / Ticket", 245, 407, 160, 34); addButton(hwnd, ID_CANCEL, "Cancel Booking", 415, 407, 125, 34); addButton(hwnd, ID_ADVISOR, "Buy Now or Wait?", 550, 407, 140, 34);
        addLabel(hwnd, "E-ticket summary", 22, 455, 150, 20);
        hTicketOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Your ticket summary will appear here after booking or checking a booking ID.", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 22, 477, 828, 150, hwnd, (HMENU)(INT_PTR)ID_TICKET_OUT, NULL, NULL);
        SendMessage(hTicketOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
        hMessage = CreateWindowA("STATIC", "Choose route and date, then search. This is a simulated academic demo; no real booking/payment occurs.", WS_CHILD | WS_VISIBLE, 22, 635, 830, 25, hwnd, NULL, NULL, NULL);
        SendMessage(hMessage, WM_SETFONT, (WPARAM)hFont, TRUE);
        // Display sample flights immediately when the application opens.
        PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_SEARCH, BN_CLICKED), 0);
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_SEARCH: performSearch(); break;
        case ID_BOOK: bookSelected(); break;
        case ID_STATUS: checkStatus(); break;
        case ID_CANCEL: cancelBooking(); break;
        case ID_ADVISOR: buyWaitAdvisor(); break;
        }
        return 0;
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wp; SetTextColor(dc, RGB(232, 238, 248)); SetBkColor(dc, RGB(16, 26, 48)); return (LRESULT)hBackground;
    }
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wp; SetTextColor(dc, RGB(25, 35, 55)); SetBkColor(dc, RGB(245, 248, 252)); return (LRESULT)hWhite;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(hwnd, &ps); RECT r; GetClientRect(hwnd, &r);
        HBRUSH brush = CreateSolidBrush(RGB(16, 26, 48)); FillRect(dc, &r, brush); DeleteObject(brush);
        SetBkMode(dc, TRANSPARENT); SetTextColor(dc, RGB(245, 190, 65)); SelectObject(dc, hBoldFont);
        TextOutA(dc, 22, 15, "SKYNOVA - SMART AIRLINE RESERVATION", 35);
        EndPaint(hwnd, &ps); return 0;
    }
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
    INITCOMMONCONTROLSEX icc; icc.dwSize = sizeof(icc); icc.dwICC = ICC_LISTVIEW_CLASSES; InitCommonControlsEx(&icc);
    hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    hBoldFont = CreateFontA(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    hBackground = CreateSolidBrush(RGB(16, 26, 48)); hWhite = CreateSolidBrush(RGB(245, 248, 252));
    const char CLASS_NAME[] = "SKYNOVA_WINDOW";
    WNDCLASSA wc = {}; wc.lpfnWndProc = WindowProcedure; wc.hInstance = instance; wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hbrBackground = hBackground; RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, CLASS_NAME, "SKYNOVA | Smart Airline Reservation System", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 890, 720, NULL, NULL, instance, NULL);
    if (!hwnd) return 0;
    ShowWindow(hwnd, show); UpdateWindow(hwnd);
    MSG msg; while (GetMessage(&msg, NULL, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessage(&msg); }
    DeleteObject(hFont); DeleteObject(hBoldFont); DeleteObject(hBackground); DeleteObject(hWhite);
    return (int)msg.wParam;
}
