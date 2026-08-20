function println(text) {
    ::print(text + "\n");
}

function brara() {
// brara <- function() { // similar, but the function name will be nullptr
    ::print("XXXXXXX lambda: this=" + this + "\n");
    return 5;
}

glubtbl <- {}
function glub[glubtbl]() {
    ::print("XXXXXXXXXXXXXX glub: " + glubtbl + " " + this + "\n");
}
function glubtbl::glub2() {
    ::print("XXXXXXXXXXXXXX glub2: " + glubtbl + " " + this + "\n");
}
glub();
glubtbl.glub2();

bar <- {}
function bar::foo(a, b, ...) {
    ::print("###### foo " + bar + " == " + this + "\n");
    ::print(vargv + "\n");
    foreach(i, v in vargv) {
        ::print(i + ". " + v + "\n");
    }
    ::print("#############\n");
}
bar.foo(5, 10, 15);

//brara <- @() 5;
if (false) {
foreach(key, val in getroottable()) {
    ::print(key + " -> " + val + "\n");
}
}

if (false) {
function foobar() {
    ::print("----------------------------\n");
    ::print("Stackinfo:\n");
    do_stacktrace();
    ::print("---------------\n");
    for(local i = 0; i < 10; ++i) {
        local si = getstackinfos(i);
        if (si == null) {
            break;
        }

        ::print("  " + i + "\n");
        foreach(key, val in si) {
            ::print("    " + key + " -> " + val + "\n");
        }
    }
    ::print("----------------------------\n")
}

foobar();
}

::print("Global table: " + this);
// ::printcallstack()
b <- (@() 5)();
::print(b + " --------\n");
::print(5 + " --------\n");

function test() {
    print("--------- end --------\n");
    a <- "defined from within a function";

    tbl <- {
        a = 5
        b = 10
    };
    //tbl.a = 5;
}

do_suspend();

//print("function: " + test + "\n");
//test();
//a = 5;
//a = 15;

::print("resume\n");
do_suspend();
::print("resume\n");
do_suspend();
::print("resume\n");
do_suspend();
::print("resume\n");

::print("---------- class testing\n");
class Foobar
{
    // member variables
    a = 10;
    b = 5;

    static c = 10;

    constructor() {
        ::print("Constructor(): " + this + "\n");

        // doesn't work, classes don't allow new slots
        // a <- 5;

        a += 10;
        b += 10;

        // Doesn't work, class variables or read-only
        // c += 10;
    }

    function foo() {
        ::print(a + " " + b + " " + c + "\n");
    }
}

foobar <- Foobar();
foobar.foo();

foobar2 <- Foobar();
foobar2.foo();

::print("\n\n");
::println("### Exception test:");
try {
    throw 5;
} catch(a) {
    ::println("caught exception: " + a);
}

/* EOF */
