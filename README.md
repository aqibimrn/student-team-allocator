# Student Team Allocator

#### Video Demo: https://youtu.be/hailh1XFddk

#### Description:

The Student Team Allocator, is a software project designed to assist tutors and educators in efficiently forming balanced student teams from a CSV dataset. The motivation behind this project was to provide an alternative to manual team allocation or reliance on third-party tools, by offering a customizable, transparent, and performance-oriented solution tailored for academic environments. The system focuses on fairness, balance, and usability, ensuring that every team is viable and meets predefined constraints, such as having at least one eligible leader.

At its core, the program reads structured student data from a CSV file and allocates students into teams using one of two algorithms, a simple greedy algorithm or a more advanced statistical algorithm that leverages variance and standard deviation. The greedy algorithm is designed for speed and simplicity, making quick allocation decisions based on available criteria, while the advanced algorithm focuses on achieving statistically balanced teams by minimizing score variance across teams. By separating these two approaches, the program allows the more computationally expensive algorithm to run only when necessary, improving overall efficiency.

The project is organized into multiple well-defined components to promote clarity, maintainability, and reusability. Input and output handling is separated from the allocation logic, ensuring that CSV parsing and file operations do not interfere with team optimization logic. The allocator module is responsible for assigning students to teams, while the optimizer module evaluates team quality and generates swap suggestions when imbalances are detected. A visualizer and user interface layer provide an interactive GUI that allows users to load CSV files, compute allocation scores, view teams, and apply suggested swaps. Finally, the main module organizes the flow of the program.

Several standard C++ libraries were used throughout the project to support these features. Libraries such as fstream and sstream were used extensively for reading and parsing CSV files, while vector and string enabled efficient data storage and manipulation. The cctype library assisted with data validation and formatting, and cmath was used for mathematical computations, particularly when calculating variance and standard deviation for the advanced allocation algorithm. These libraries collectively enabled robust data handling and precise statistical analysis.

One key design decision was the implementation of swap optimization without full recomputation. When a swap between two students is applied, only the affected teams have their statistics recalculated, rather than recomputing metrics for all teams. This significantly reduces computational overhead, especially for large class sizes. Additionally, the suggestion system limits its scope by focusing only on teams with extreme or problematic values, further improving performance. Once a suggested swap is applied, suggestions are locked to prevent conflicting recommendations, ensuring consistency and clarity for the user.

The graphical user interface plays a major role in making the system accessible and intuitive. Users can visually inspect teams through dynamically loaded team cards, view statistics panels that explain why certain suggestions are made, and interact with suggestion boxes that translate algorithmic output into human-readable explanations. Thoughtful use of colors, scrolling, feedback animations, and visual indicators (such as highlighting teams missing eligible leaders) enhances the overall user experience and reflects a strong emphasis on creative and user-centered design.

From an architectural perspective, efficiency was a primary consideration. Functions and header files are modular and reusable. Rendering optimizations ensure that only visible UI components are drawn on screen, conserving resources. These choices demonstrate an analytical approach to both software design and performance optimization.

In terms of ownership and future direction, this project reflects a high level of independence and responsibility. I designed and implemented both the algorithmic backend (including score calculations, leader handling, team allocation, and swap optimization) and the interactive GUI. Future improvements could include integrating artificial intelligence techniques to generate smarter swap suggestions, implementing undo/redo functionality for swaps, exporting allocated teams back into a CSV file, and adding charts or animations to further improve usability and insight. Time constraints prevented these features from being fully realized, but they represent clear and valuable directions for further development.

Overall, the Student Team Allocator is a complete, efficient, and creative solution that combines algorithmic sophistication with practical usability. It demonstrates strong software engineering principles and provides a meaningful tool that educators can rely on for fair and transparent team allocation.

#### References:

Generative AI Acknowledgement:
The use of ChatGPT was limited to generating a sample CSV dataset used for testing and demonstration purposes. This sample data file is included in the project folder and was not used in the implementation of the allocation algorithms themselves.

