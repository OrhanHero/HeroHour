open HeroHour.Prototype
open HeroHour.Prototype.PrototypeContent

let private fail message =
    eprintfn "FAILED: %s" message
    1

[<EntryPoint>]
let main _ =
    let validationErrors = validateScope scope

    if not validationErrors.IsEmpty then
        validationErrors |> List.iter (eprintfn "FAILED: %s")
        1
    elif unitsFor Hero |> List.length <> 3 then
        fail "HERO must have three prototype units."
    elif unitsFor USA |> List.length <> 3 then
        fail "USA must have three prototype units."
    else
        printfn "Prototype scope valid: 1 map, 2 factions, 6 units."
        0
