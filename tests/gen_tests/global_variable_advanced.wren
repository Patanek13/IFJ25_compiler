import "ifj25" for Ifj

class Program {
    // Funkce pro modifikaci globalni promenne (Int)
    static incrementGlobal() {
        // Precte __gCounter, pripocte 1 a ulozi zpet
        __gCounter = __gCounter + 1
        Ifj.write("Inside function: ")
        Ifj.write(__gCounter)
        Ifj.write("\n")
    }

    // Funkce pro nastaveni globalniho stringu
    static setGlobalMsg() {
        __gMessage = "Globalni retezec z funkce"
    }

    // Rekurzivni funkce vyuzivajici globalni promennou jako citac hloubky
    static diveDeep(n) {
        if (n > 0) {
            // Zvysime globalni citac
            __gDepth = __gDepth + 1
            var next
            next = n - 1
            __dummy = diveDeep(next)
        } else {
        }
    }

    static main() {
        // 1. Inicializace a vypis (Int)
        __gCounter = 10
        Ifj.write("Start main: ")
        Ifj.write(__gCounter)
        Ifj.write("\n")

        // 2. Modifikace ve funkci
        __dummy = incrementGlobal()

        Ifj.write("Back in main: ")
        Ifj.write(__gCounter)
        Ifj.write("\n")

        // 3. Zmena typu (Int -> String) a nova globalni
        __dummy = setGlobalMsg()
        Ifj.write("Global Message: ")
        Ifj.write(__gMessage)
        Ifj.write("\n")

        // 4. Interakce s lokalni promennou
        var localCopy
        localCopy = __gCounter // Ulozime si 11
        __gCounter = 0         // Vynulujeme globalni

        Ifj.write("Local copy: ")
        Ifj.write(localCopy)
        Ifj.write("\n")
        Ifj.write("Global reset: ")
        Ifj.write(__gCounter)
        Ifj.write("\n")

        // 5. Globalni promenna v rekurzi
        __gDepth = 0
        Ifj.write("Recursion start depth: ")
        Ifj.write(__gDepth)
        Ifj.write("\n")

        __dummy = diveDeep(5) // Mela by zvysit __gDepth 5krat

        Ifj.write("Recursion end depth: ")
        Ifj.write(__gDepth)
        Ifj.write("\n")
    }
}
