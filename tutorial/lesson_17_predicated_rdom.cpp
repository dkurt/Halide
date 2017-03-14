// Halide tutorial lesson 17: Reductions over non-rectangular domain

// This lesson demonstrates how to define iterations over a non-rectangular domain

// On linux, you can compile and run it like so:
// g++ lesson_17*.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o lesson_17 -std=c++11
// LD_LIBRARY_PATH=../bin ./lesson_17

// On os x:
// g++ lesson_17*.cpp -g -I ../include -L ../bin -lHalide -o lesson_17 -std=c++11
// DYLD_LIBRARY_PATH=../bin ./lesson_17

// If you have the entire Halide source tree, you can also build it by
// running:
//    make tutorial_lesson_17_predicated_rdom
// in a shell with the current directory at the top of the halide
// source tree.

// The only Halide header file you need is Halide.h. It includes all of Halide.
#include "Halide.h"

// We'll also include stdio for printf.
#include <stdio.h>

using namespace Halide;

int main(int argc, char **argv) {

    // In lesson 9, we learned how to use RDom to define a "reduction domain" in
    // Halide update definitions. The domain defined by RDom, however, is
    // always rectangular. In some cases, we might want to iterate over some
    // non-rectangular domain, e.g. a circle. We can achieve this behavior by
    // using the RDom::where directive.

    {
        // Starting with this pure definition:
        Func f("circle");
        Var x("x"), y("y");
        f(x, y) = x + y;

        // Say we want an update that squares the values inside a circular region
        // centered at (3, 3) with radius of 3. To do this, we first define
        // the minimal bounding box over the circular region using RDom.
        RDom r(0, 7, 0, 7, "r");
        // The bounding box does not have to be minimal. In fact, the box can be
        // of any size, as long it covers the region we'd like to update. However,
        // the tighter the bounding box, the tighter the generated loop bounds
        // will be. Halide will tighten the loop bounds automatically when
        // possible, but in general, it is better to define a minimal bounding
        // box.

        // Then, we use RDom::where to define the predicate over that bounding box,
        // such that the update is performed only if the given predicate evaluates
        // to true, i.e. within the circular region.
        r.where((r.x - 3)*(r.x - 3) + (r.y - 3)*(r.y - 3) <= 10);
        // After defining the predicate, we then define the update.
        f(r.x, r.y) *= 2;

        Buffer<int> halide_result = f.realize(7, 7);

        // See figures/lesson_17_rdom_circular.gif for a visualization of
        // what this did.

        // The equivalent C is:
        int c_result[7][7];
        for (int y = 0; y < 7; y++) {
            for (int x = 0; x < 7; x++) {
                c_result[y][x] = x + y;
            }
        }
        for (int r_y = 0; r_y < 7; r_y++) {
            for (int r_x = 0; r_x < 7; r_x++) {
                // Update is only performed if the predicate evaluates to true.
                if ((r_x - 3)*(r_x - 3) + (r_y - 3)*(r_y - 3) <= 10) {
                    c_result[r_y][r_x] *= 2;
                }
            }
        }

        // Check the results match:
        for (int y = 0; y < 7; y++) {
            for (int x = 0; x < 7; x++) {
                if (halide_result(x, y) != c_result[y][x]) {
                    printf("halide_result(%d, %d) = %d instead of %d\n",
                           x, y, halide_result(x, y), c_result[y][x]);
                    return -1;
                }
            }
        }
    }

    {
        // We can also define multiple predicates over an RDom. Let's say now
        // we want the update to happen within some triangular region. To do
        // this, we need to define three predicates, where each corresponds to
        // one side of the triangle.
        Func f("triangle");
        Var x("x"), y("y");
        f(x, y) = x + y;
        // First, let's define the minimal bounding box over the triangular
        // region.
        RDom r(0, 8, 0, 10, "r");
        // Next, let's add the first predicate to the RDom using the RDom::where
        // directive.
        r.where(r.x + r.y > 5);
        // Add the 2nd and the 3rd predicates.
        r.where(3*r.y - 2*r.x < 15);
        r.where(4*r.x - r.y < 20);

        // We can also pack the multiple predicates into one like so:
        // r.where((r.x + r.y > 5) && (3*r.y - 2*r.x < 15) && (4*r.x - r.y < 20));

        // Then define the update.
        f(r.x, r.y) *= 2;

        Buffer<int> halide_result = f.realize(10, 10);

        // See figures/lesson_17_rdom_triangular.gif for a visualization of
        // what this did.

        // The equivalent C is:
        int c_result[10][10];
        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                c_result[y][x] = x + y;
            }
        }
        for (int r_y = 0; r_y < 10; r_y++) {
            for (int r_x = 0; r_x < 8; r_x++) {
                // Update is only performed if the predicate evaluates to true.
                if ((r_x + r_y > 5) && (3*r_y - 2*r_x < 15) && (4*r_x - r_y < 20)) {
                    c_result[r_y][r_x] *= 2;
                }
            }
        }

        // Check the results match:
        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                if (halide_result(x, y) != c_result[y][x]) {
                    printf("halide_result(%d, %d) = %d instead of %d\n",
                           x, y, halide_result(x, y), c_result[y][x]);
                    return -1;
                }
            }
        }
    }

    {
        // The predicate is not limited to RDom's variables only (e.g. r.x, r.y, etc.).
        // It could refer to free variables in the update definition or calls to
        // other Funcs, or make recursive calls to the same Func. Here is some
        // example:
        Func f("call_f"), g("call_g");
        Var x("x"), y("y");
        f(x, y) = 2 * x + y;
        g(x, y) = x + y;

        // This RDom's predicates depend on the initial value of 'f'.
        RDom r1(0, 5, 0, 5, "r1");
        r1.where(f(r1.x, r1.y) >= 4);
        r1.where(f(r1.x, r1.y) <= 7);
        f(r1.x, r1.y) += 1;
        f.compute_root();

        // While this one involves calls to another Func.
        RDom r2(1, 3, 1, 3, "r2");
        r2.where(f(r2.x, r2.y) < 15);
        g(r2.x, r2.y) += f(r2.x, r2.y)/2;

        Buffer<int> halide_result_f = f.realize(5, 5);
        Buffer<int> halide_result_g = g.realize(5, 5);

        // See figures/lesson_17_rdom_calls_in_predicate.gif for a visualization
        // of what this did.

        // The equivalent C for 'f' is:
        int c_result_f[5][5];
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                c_result_f[y][x] = 2 * x + y;
            }
        }
        for (int r1_y = 0; r1_y < 5; r1_y++) {
            for (int r1_x = 0; r1_x < 5; r1_x++) {
                // Update is only performed if the predicate evaluates to true.
                if ((c_result_f[r1_y][r1_x] >= 10) && (c_result_f[r1_y][r1_x] != 15)) {
                    c_result_f[r1_y][r1_x] += 1;
                }
            }
        }

        // And, the equivalent C for 'g' is:
        int c_result_g[5][5];
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                c_result_g[y][x] = x + y;
            }
        }
        for (int r2_y = 1; r2_y < 4; r2_y++) {
            for (int r1_x = 1; r1_x < 4; r1_x++) {
                // Update is only performed if the predicate evaluates to true.
                if (c_result_f[r2_y][r1_x] < 15) {
                    c_result_g[r2_y][r1_x] += c_result_f[r2_y][r1_x]/2;
                }
            }
        }

        // Check the results match:
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                if (halide_result_f(x, y) != c_result_f[y][x]) {
                    printf("halide_result_f(%d, %d) = %d instead of %d\n",
                           x, y, halide_result_f(x, y), c_result_f[y][x]);
                    return -1;
                }
            }
        }
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                if (halide_result_g(x, y) != c_result_g[y][x]) {
                    printf("halide_result_g(%d, %d) = %d instead of %d\n",
                           x, y, halide_result_g(x, y), c_result_g[y][x]);
                    return -1;
                }
            }
        }
    }

    printf("Success!\n");

    return 0;
}
