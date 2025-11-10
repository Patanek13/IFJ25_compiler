import "ifj25" for Ifj

class Program {

    // Globalni promenna (pro prirazeni navratovych hodnot)
    // Komentare: '//' radkovy a '/*' blokovy '*/'
    __g = null

    // --- Definice funkci ---

    // 1. Funkce bez parametru a s navratem
    static test_return_val() {
        return 10 // return <term>
    }

    // 2. Funkce s prazdnym returnem (EXTSTAT)
    static test_return_empty() {
        return // return
    }

    // 3. Funkce s parametry
    static test_params(a, b) {
        __g = Ifj.write(a)
        __g = Ifj.write(b)
        return a
    }

    // 4. Getter
    static test_getter {
        return "getter_val"
    }

    // 5. Setter
    static test_setter = (val) {
        __g = val // prirazeni parametru
    }

    // --- Hlavni funkce ---
    static main() {

        // 6. Definice lokalni promenne
        var x
        var y

        // 7. Prirazeni (literaly)
        x = 1          // Integer
        x = 10.5       // Float
        x = "hello"    // String
        x = null       // Null

        // 8. Prirazeni (globalni promenna)
        __g = x

        // 9. Podmineny prikaz IF-ELSE
        if (x) {
            y = 1
        } else {
            y = 2
        }

        // 10. Cyklus WHILE
        while (y) {
            y = null // ukonceni cyklu
        }

        // 11. Volani vestavenych funkci (prirazeni)
        x = Ifj.read_num()
        y = Ifj.read_str()
        __g = Ifj.write(x)
        __g = Ifj.write(y)

        // 12. Volani uzivatelskych funkci (prirazeni)
        x = test_return_val()
        y = test_return_empty() // y bude null
        x = test_params(x, y)

        // 13. Pouziti Getteru (prirazeni)
        x = test_getter

        // 14. Pouziti Setteru (prirazeni)
        test_setter = x

        // 15. Samostatny blok
        {
            var z // nova promenna ve vnitrnim scope
            z = 100
            __g = Ifj.write(z)
        }

        return // Konec main
    }
}
