(** [binop] binary operations *)
type binop =
  [ `Add
  | `Minus
  | `Mult
  | `Div
  ]

(** [unop] unary operations *)
type unop = [ `UMinus ]

(** [stype] static types *)
type stype =
  | STInt
  | STFloat

(** [expr] expression types *)
type expr =
  | Int of int
  | Float of float
  | Var of string
  | Let of string * stype * expr
  | Binop of binop * expr * expr
  | Unop of unop * expr
