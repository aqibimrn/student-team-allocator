// importing the libraries
#include "structs.h"
#include "splashkit.h"
#include "ui.h"
#include <string>
#include <sstream>

// main function
int main()
{
    // create UIContext object
    UIContext ctx;

    // intialize the UI interface
    ui_init(ctx);
    ui_run(ctx);
    ui_cleanup(ctx);

// return 0 to indicate a successful program execution!
    return 0;
}
