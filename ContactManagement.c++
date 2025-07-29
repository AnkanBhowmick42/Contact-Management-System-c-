#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <ctime>
#include <sys/stat.h>
using namespace std;

// Function to check if file exists
inline bool fileExists(const string &filename)
{
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

// Contact class to store individual contact information
class Contact
{
private:
    string name;
    string phone;
    string email;
    string address;
    string birthday;
    string notes;
    string category; // Personal, Work, Family, etc.

public:
    Contact(string n = "", string p = "", string e = "", string a = "", string b = "", string nt = "", string c = "Personal")
        : name(n), phone(p), email(e), address(a), birthday(b), notes(nt), category(c) {}

    // Getters
    string getName() const { return name; }
    string getPhone() const { return phone; }
    string getEmail() const { return email; }
    string getAddress() const { return address; }
    string getBirthday() const { return birthday; }
    string getNotes() const { return notes; }
    string getCategory() const { return category; }

    // Setters
    void setName(string n) { name = n; }
    void setPhone(string p) { phone = p; }
    void setEmail(string e) { email = e; }
    void setAddress(string a) { address = a; }
    void setBirthday(string b) { birthday = b; }
    void setNotes(string n) { notes = n; }
    void setCategory(string c) { category = c; }

    // Validate phone number format
    static bool isValidPhone(const string &phone)
    {
        try
        {
            regex phonePattern(R"(^\+?[1-9]\d{7,14}$)");
            return regex_match(phone, phonePattern);
        }
        catch (const regex_error &e)
        {
            cerr << "Regex error: " << e.what() << endl;
            return false;
        }
    }

    // Validate email format
    static bool isValidEmail(const string &email)
    {
        try
        {
            regex emailPattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
            return regex_match(email, emailPattern);
        }
        catch (const regex_error &e)
        {
            cerr << "Regex error: " << e.what() << endl;
            return false;
        }
    }

    // Validate date format (DD/MM/YYYY)
    static bool isValidDate(const string &date)
    {
        try
        {
            if (date.empty())
                return true; // Allow empty dates

            regex datePattern(R"(^(0[1-9]|[12][0-9]|3[01])/(0[1-9]|1[0-2])/\d{4}$)");
            if (!regex_match(date, datePattern))
                return false;

            // Extract day, month, year
            int day = stoi(date.substr(0, 2));
            int month = stoi(date.substr(3, 2));
            int year = stoi(date.substr(6, 4));

            // Check year is not in future
            time_t now = time(nullptr);
            tm *currentTime = localtime(&now);
            if (year > (currentTime->tm_year + 1900))
                return false;

            // Check days in month
            int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
                daysInMonth[1] = 29; // Leap year

            return day <= daysInMonth[month - 1];
        }
        catch (const exception &e)
        {
            cerr << "Date validation error: " << e.what() << endl;
            return false;
        }
    }

    // Validate category
    static bool isValidCategory(const string &category)
    {
        vector<string> validCategories = {"Personal", "Work", "Family", "Other"};
        return find(validCategories.begin(), validCategories.end(), category) != validCategories.end();
    }

    // Display contact details
    void display() const
    {
        cout << "\n--- Contact Details ---" << endl;
        cout << "Name     : " << name << endl;
        cout << "Category : " << category << endl;
        cout << "Phone    : " << phone << endl;
        cout << "Email    : " << email << endl;
        cout << "Address  : " << address << endl;
        cout << "Birthday : " << birthday << endl;
        cout << "Notes    : " << notes << endl;
        cout << "--------------------" << endl;
    }
};

// ContactManager class to manage all contacts
class ContactManager
{
private:
    vector<Contact> contacts;
    const string filename = "contacts.dat";

    void saveToFile()
    {
        try
        {
            ofstream file(filename, ios::binary);
            if (!file)
            {
                throw runtime_error("Could not open file for writing: " + filename);
            }

            // Write number of contacts first
            size_t numContacts = contacts.size();
            file.write(reinterpret_cast<char *>(&numContacts), sizeof(numContacts));

            for (const Contact &contact : contacts)
            {
                vector<string> fields = {
                    contact.getName(), contact.getPhone(), contact.getEmail(),
                    contact.getAddress(), contact.getBirthday(), contact.getNotes(),
                    contact.getCategory()};

                for (const string &field : fields)
                {
                    size_t len = field.length();
                    file.write(reinterpret_cast<char *>(&len), sizeof(len));
                    file.write(field.c_str(), len);
                }

                if (!file)
                {
                    throw runtime_error("Error writing to file: " + filename);
                }
            }
            file.close();
            cout << "Data saved successfully!" << endl;
        }
        catch (const exception &e)
        {
            cerr << "Error saving to file: " << e.what() << endl;
            cerr << "Your changes may not have been saved!" << endl;
        }
    }

    void loadFromFile()
    {
        try
        {
            ifstream file(filename, ios::binary);
            if (!file)
            {
                // Don't show error for first time use
                if (fileExists(filename))
                {
                    throw runtime_error("Could not open file for reading: " + filename);
                }
                return;
            }

            contacts.clear();

            // Read number of contacts
            size_t numContacts;
            file.read(reinterpret_cast<char *>(&numContacts), sizeof(numContacts));

            for (size_t i = 0; i < numContacts; i++)
            {
                vector<string> fields(7); // 7 fields per contact

                for (string &field : fields)
                {
                    size_t len;
                    if (!file.read(reinterpret_cast<char *>(&len), sizeof(len)))
                    {
                        throw runtime_error("Error reading field length");
                    }

                    field.resize(len);
                    if (!file.read(&field[0], len))
                    {
                        throw runtime_error("Error reading field data");
                    }
                }

                contacts.emplace_back(
                    fields[0], fields[1], fields[2], fields[3],
                    fields[4], fields[5], fields[6]);
            }

            file.close();
            cout << "Loaded " << contacts.size() << " contacts successfully!" << endl;
        }
        catch (const exception &e)
        {
            cerr << "Error loading from file: " << e.what() << endl;
            cerr << "The contacts file might be corrupted. Creating backup..." << endl;

            // Create backup of potentially corrupted file
            try
            {
                if (fileExists(filename))
                {
                    string backupFile = filename + ".backup";
                    ifstream src(filename, ios::binary);
                    ofstream dst(backupFile, ios::binary);
                    dst << src.rdbuf();
                    cout << "Backup created as " << backupFile << endl;
                }
            }
            catch (const exception &e)
            {
                cerr << "Failed to create backup: " << e.what() << endl;
            }

            contacts.clear(); // Start fresh
        }
    }

public:
    ContactManager()
    {
        loadFromFile();
    }

    ~ContactManager()
    {
        saveToFile();
    }

    void addContact()
    {
        try
        {
            string name, phone, email, address, birthday, notes, category;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            do
            {
                cout << "Enter Name: ";
                getline(cin, name);
                if (name.empty())
                {
                    cout << "Name cannot be empty! Please try again." << endl;
                }
            } while (name.empty());

            do
            {
                cout << "Enter Phone Number (E.g., +1234567890): ";
                getline(cin, phone);
                if (!Contact::isValidPhone(phone))
                {
                    cout << "Invalid phone number format! Please enter 8-15 digits with optional '+' at start." << endl;
                }
            } while (!Contact::isValidPhone(phone));

            do
            {
                cout << "Enter Email: ";
                getline(cin, email);
                if (!Contact::isValidEmail(email))
                {
                    cout << "Invalid email format! Please enter a valid email address (e.g., user@domain.com)." << endl;
                }
            } while (!Contact::isValidEmail(email));

            cout << "Enter Address (optional): ";
            getline(cin, address);

            do
            {
                cout << "Enter Birthday (DD/MM/YYYY) (optional - press Enter to skip): ";
                getline(cin, birthday);
                if (!birthday.empty() && !Contact::isValidDate(birthday))
                {
                    cout << "Invalid date format! Please use DD/MM/YYYY format or leave empty." << endl;
                }
            } while (!birthday.empty() && !Contact::isValidDate(birthday));

            cout << "Enter Notes (optional): ";
            getline(cin, notes);

            do
            {
                cout << "Enter Category (Personal/Work/Family/Other): ";
                getline(cin, category);
                if (category.empty())
                {
                    category = "Personal";
                    break;
                }
                if (!Contact::isValidCategory(category))
                {
                    cout << "Invalid category! Please choose from: Personal, Work, Family, or Other." << endl;
                }
            } while (!Contact::isValidCategory(category));

            contacts.emplace_back(name, phone, email, address, birthday, notes, category);
            cout << "Contact added successfully!" << endl;
            saveToFile();
        }
        catch (const exception &e)
        {
            cerr << "Error adding contact: " << e.what() << endl;
        }
    }

    void viewContacts() const
    {
        if (contacts.empty())
        {
            cout << "No contacts found!" << endl;
            return;
        }

        cout << "\nContact List:" << endl;
        cout << string(75, '-') << endl;
        cout << left << setw(30) << "Name"
             << setw(15) << "Phone"
             << setw(30) << "Email" << endl;
        cout << string(75, '-') << endl;

        for (const Contact &contact : contacts)
        {
            contact.display();
        }
        cout << string(75, '-') << endl;
    }

    void searchContact() const
    {
        if (contacts.empty())
        {
            cout << "No contacts to search!" << endl;
            return;
        }

        string searchTerm;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter name to search: ";
        getline(cin, searchTerm);

        bool found = false;
        for (const Contact &contact : contacts)
        {
            if (contact.getName().find(searchTerm) != string::npos)
            {
                if (!found)
                {
                    cout << "\nSearch Results:" << endl;
                    cout << string(75, '-') << endl;
                    cout << left << setw(30) << "Name"
                         << setw(15) << "Phone"
                         << setw(30) << "Email" << endl;
                    cout << string(75, '-') << endl;
                }
                contact.display();
                found = true;
            }
        }

        if (!found)
        {
            cout << "No matching contacts found!" << endl;
        }
    }

    void editContact()
    {
        if (contacts.empty())
        {
            cout << "No contacts to edit!" << endl;
            return;
        }

        string searchName;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter name of contact to edit: ";
        getline(cin, searchName);

        for (Contact &contact : contacts)
        {
            if (contact.getName() == searchName)
            {
                string name, phone, email;

                cout << "Enter new name (press enter to keep current): ";
                getline(cin, name);
                if (!name.empty())
                    contact.setName(name);

                cout << "Enter new phone (press enter to keep current): ";
                getline(cin, phone);
                if (!phone.empty())
                    contact.setPhone(phone);

                cout << "Enter new email (press enter to keep current): ";
                getline(cin, email);
                if (!email.empty())
                    contact.setEmail(email);

                cout << "Contact updated successfully!" << endl;
                saveToFile();
                return;
            }
        }
        cout << "Contact not found!" << endl;
    }

    void deleteContact()
    {
        if (contacts.empty())
        {
            cout << "No contacts to delete!" << endl;
            return;
        }

        string searchName;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter name of contact to delete: ";
        getline(cin, searchName);

        for (auto it = contacts.begin(); it != contacts.end(); ++it)
        {
            if (it->getName() == searchName)
            {
                cout << "Are you sure you want to delete this contact? (y/n): ";
                string confirm;
                getline(cin, confirm);
                if (confirm == "y" || confirm == "Y")
                {
                    contacts.erase(it);
                    cout << "Contact deleted successfully!" << endl;
                    saveToFile();
                }
                else
                {
                    cout << "Deletion cancelled." << endl;
                }
                return;
            }
        }
        cout << "Contact not found!" << endl;
    }

    // Sort contacts by name
    void sortContacts()
    {
        sort(contacts.begin(), contacts.end(),
             [](const Contact &a, const Contact &b)
             {
                 return a.getName() < b.getName();
             });
        cout << "Contacts sorted by name!" << endl;
        saveToFile();
    }

    // Filter contacts by category
    void filterByCategory() const
    {
        if (contacts.empty())
        {
            cout << "No contacts to filter!" << endl;
            return;
        }

        string category;
        cout << "Enter category to filter (Personal/Work/Family/Other): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, category);

        bool found = false;
        for (const Contact &contact : contacts)
        {
            if (contact.getCategory() == category)
            {
                if (!found)
                {
                    cout << "\nContacts in category '" << category << "':" << endl;
                    found = true;
                }
                contact.display();
            }
        }

        if (!found)
        {
            cout << "No contacts found in category '" << category << "'!" << endl;
        }
    }

    // Export contacts to CSV file
    void exportToCSV() const
    {
        ofstream file("contacts.csv");
        if (!file)
        {
            cout << "Error: Could not create CSV file!" << endl;
            return;
        }

        // Write CSV header
        file << "Name,Category,Phone,Email,Address,Birthday,Notes\n";

        // Write contact data
        for (const Contact &contact : contacts)
        {
            file << contact.getName() << ","
                 << contact.getCategory() << ","
                 << contact.getPhone() << ","
                 << contact.getEmail() << ","
                 << contact.getAddress() << ","
                 << contact.getBirthday() << ","
                 << contact.getNotes() << "\n";
        }

        file.close();
        cout << "Contacts exported to 'contacts.csv' successfully!" << endl;
    }
};

// Main menu function
void displayMenu()
{
    cout << "\n=== Contact Management System ===" << endl;
    cout << "1.  Add Contact" << endl;
    cout << "2.  View All Contacts" << endl;
    cout << "3.  Search Contact" << endl;
    cout << "4.  Edit Contact" << endl;
    cout << "5.  Delete Contact" << endl;
    cout << "6.  Sort Contacts by Name" << endl;
    cout << "7.  Filter Contacts by Category" << endl;
    cout << "8.  Export Contacts to CSV" << endl;
    cout << "9.  Exit" << endl;
    cout << "=============================" << endl;
    cout << "Enter your choice (1-9): ";
};

int main()
{
    ContactManager manager;
    int choice;

    cout << "Welcome to Contact Management System" << endl;
    cout << "Version 2.0 - Enhanced Edition" << endl;

    while (true)
    {
        displayMenu();
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a number." << endl;
            continue;
        }

        switch (choice)
        {
        case 1:
            manager.addContact();
            break;
        case 2:
            manager.viewContacts();
            break;
        case 3:
            manager.searchContact();
            break;
        case 4:
            manager.editContact();
            break;
        case 5:
            manager.deleteContact();
            break;
        case 6:
            manager.sortContacts();
            break;
        case 7:
            manager.filterByCategory();
            break;
        case 8:
            manager.exportToCSV();
            break;
        case 9:
            cout << "\nThank you for using Contact Management System!" << endl;
            cout << "Goodbye!" << endl;
            return 0;
        default:
            cout << "Invalid choice! Please try again." << endl;
        }
    }

    return 0;
}
