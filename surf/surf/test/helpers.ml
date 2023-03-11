open Surf

(** [check_interp name i s env] sets up an alcotest case for the interp function with name
    [name] evaluating [i] against the interpreted [s] *)
let check_interp name i s env =
  Alcotest.(check bool)
    name
    true
    (String.equal (string_of_int i) (Interpreter.interp_ret env s))
;;
