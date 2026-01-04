#include "io.h"
#include "splashkit.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>


using std::vector;
using std::ifstream;
using std::stringstream;
using std::string;

/**
 * trim the space between the two ends
 */
std::string trim_string(const std::string &s)
{
    int start = 0;
    int end = s.size() - 1;

    // move start forward while current char is whitespace
    while (start <= end && std::isspace(s[start]))
    {
        start++;
    }

    // similarly move end backward while current char is whitespace
    while (end >= start && std::isspace(s[end]))
    {
        end--;
    }

    // if the string is all whitespace return the original, empty string
    if (end < start) 
    {
        return std::string();
    }

    // return a substring from start to the end
    return s.substr(start, end - start + 1);
}

/**
 * convert string to integer using catch if it fails to avoid crashing
 */
int safe_stoi(const std::string &token, int fallback)
{
    std::string t = trim_string(token); // trim first

    // error handling
    if (t.empty())
    {
        return fallback;
    }

    // using catch as taught in programmers.guide
    try
    {
        size_t idx = 0;
        long value = std::stol(t, &idx);
        if (idx != t.size()) 
        {
            return fallback;
        }

        return value;
    }

    catch (...)
    {
        return fallback; 
    }
}

/**
 * load students from CSV file
 */
std::vector<student> load_students_from_csv(const std::string &filename)
{
    std::vector<student> students;
    std::ifstream file(filename.c_str());

    // error handling
    if (!file.is_open())
    {
        write_line("Error: Could not open file: " + filename);
        return students;
    }

    // each line from file is held in 'line'
    std::string line;
    // which line we're on
    int line_no = 1;

    // Skip the first line which is the headers
    std::getline(file, line);

    // read file line by line
    while (std::getline(file, line))
    {
        // trim the whitespace around the line
        std::string raw = trim_string(line);

        // skip the line if its blank
        if (raw.empty())
        {
            line_no++;
            continue;
        }

        // create string stream to separate values between commas
        std::stringstream ss(line);
        student s;
        std::string token;

        // separate names
        // skip the line if its missing data (error handling)
        if (!std::getline(ss, token, ','))
        {
            write_line("Skipping skewed CSV line " + std::to_string(line_no));
            line_no++;
            continue;
        }
        // store the student's name (trimmed)
        s.name = trim_string(token);

        // store student's leadership score

        if (!std::getline(ss, token, ',')) 
        {
            // error handling, if leadership score is missing then default to 0
            s.leadership = 0; 
        }
        else 
        {
            // convert safely to integer
            s.leadership = stoi(token, 0);
        }

        // frontend score
        if (!std::getline(ss, token, ',')) 
        {
            // error handling
            s.frontend = 0;
        }
        else 
        {
            // convert integer safely
            s.frontend = stoi(token, 0);
        }

        // backend scroe
        if (!std::getline(ss, token, ',')) 
        {
            s.backend = 0;
        }
        else 
        {
            s.backend = stoi(token, 0);
        }

        // security score
        if (!std::getline(ss, token, ',')) 
        {
            s.security   = 0;
        }
        else
        {
            s.security   = stoi(token, 0);
        } 

        // ui/ux score
        if (!std::getline(ss, token, ','))
        {
            s.ui = 0;
        }  
        else 
        {
            s.ui = stoi(token, 0);
        }

        // english score
        if (!std::getline(ss, token, ',')) 
        {
            s.english = 0;
        } 
        else 
        {
            s.english = stoi(token, 0);
        }
        
        // intializing values
        s.student_score = 0;
        s.x = 0.0;
        s.y = 0.0;
        s.selected = false;

        students.push_back(s);
        line_no++;
    }

    // success message & close the file

    file.close();
    write_line("Loaded " + std::to_string(students.size()) + " students from " + filename);
    return students;
}
