import "ifj25" for Ifj
class Program {

    // Globalni promenna pro prirazovani
    __d = null

    // Pomocna funkce pro test volani ve vyrazu
    static test_func(a) {
        return a + 1
    }

    // Hlavni funkce main
    static main() {

        var x
        var y
        var z

        // --- 1. Priority a asociativita (+, -, *, /) ---
        x = 1 + 2 * 3 - 10 / 5
        __d = Ifj.write("Test 1 (Priority 1 + 2*3 - 10/5): ")
        __d = Ifj.write(x) // Ocekava se: 5
        __d = Ifj.write("\n")

        // --- 2. Závorky ---
        x = (1 + 2) * 3 - (10 / 5)
        __d = Ifj.write("Test 2 (Zavorky (1+2)*3 - (10/5)): ")
        __d = Ifj.write(x) // Ocekava se: 7
        __d = Ifj.write("\n")

        // --- 3. Unární mínus ---
        x = -10 + (5 * 2)
        __d = Ifj.write("Test 3 (Unarni minus -10 + (5*2)): ")
        __d = Ifj.write(x) // Ocekava se: 0
        __d = Ifj.write("\n")

        x = 10 + -5
        __d = Ifj.write("Test 4 (Unarni minus 10 + -5): ")
        __d = Ifj.write(x) // Ocekava se: 5
        __d = Ifj.write("\n")

        // --- 4. Porovnávací operátory (<, >, <=, >=) ---
        if (10 > 5) {
            __d = Ifj.write("OK: 10 > 5\n")
        } else {
        }
        if (5 < 10) {
            __d = Ifj.write("OK: 5 < 10\n")
        } else {
        }
        if (10 >= 10) {
            __d = Ifj.write("OK: 10 >= 10\n")
        } else {
        }
        if (10 <= 10) {
            __d = Ifj.write("OK: 10 <= 10\n")
        } else {
        }
        if (1 + 1 > 3 - 2) {
            __d = Ifj.write("OK: (1+1) > (3-2)\n")
        } else {
        }

        // --- 5. Operátor 'is' ---
        y = 10
        if (y is Num) {
            __d = Ifj.write("OK: y is Num\n")
        } else {
        }
        if ("hello" is String) {
            __d = Ifj.write("OK: 'hello' is String\n")
        } else {
        }

        // --- 6. Rovnost (==, !=) ---
        if (y == 10) {
            __d = Ifj.write("OK: y == 10\n")
        } else {
        }
        if (y != 11) {
            __d = Ifj.write("OK: y != 11\n")
        } else {
        }
        if (null == null) {
            __d = Ifj.write("OK: null == null\n")
        } else {
        }
        if ("a" != "b") {
            __d = Ifj.write("OK: 'a' != 'b'\n")
        } else {
        }

        // --- 7. Logické operátory (!, &&, ||) ---
        if (true && true) {
            __d = Ifj.write("OK: true && true\n")
        } else {
        }
        if (true || false) {
            __d = Ifj.write("OK: true || false\n")
        } else {
        }
        if (!false) {
            __d = Ifj.write("OK: !false\n")
        } else {
        }

        // --- 8. Slozitejsi logicky vyraz ---
        if ((10 > 5 && "a" == "b") || !(null == 1)) {
            __d = Ifj.write("OK: Slozeny logicky vyraz\n")
        } else {
        }

        // --- 9. Ternární operátor (?) ---
        x = (1 > 0) ? 100 : 200
        __d = Ifj.write("Test 5 (Ternary 1>0 ? 100:200): ")
        __d = Ifj.write(x) // Ocekava se: 100
        __d = Ifj.write("\n")

        x = (1 < 0) ? 100 : 200
        __d = Ifj.write("Test 6 (Ternary 1<0 ? 100:200): ")
        __d = Ifj.write(x) // Ocekava se: 200
        __d = Ifj.write("\n")

        // --- 10. Volání funkcí ve výrazu ---
        x = test_func(5) // Ocekava se 6
        x = x + test_func(10) // Ocekava se 6 + 11 = 17
        __d = Ifj.write("Test 7 (Volani funkce 6 + test_func(10)): ")
        __d = Ifj.write(x) // Ocekava se: 17
        __d = Ifj.write("\n")

        x = Ifj.length("hello") + 1
        __d = Ifj.write("Test 8 (Vestavena fce Ifj.length('hello') + 1): ")
        __d = Ifj.write(x) // Ocekava se: 6
        __d = Ifj.write("\n")

        // --- 11. While smycka s vyrazem ---
        __d = Ifj.write("Test 9 (While x > 15):\n")
        while (x > 15) {
            __d = Ifj.write("  x = ")
            __d = Ifj.write(x)
            __d = Ifj.write("\n")
            x = x - 1
        } // Melo by se vypsat 17 a 16

        return
    }
}
