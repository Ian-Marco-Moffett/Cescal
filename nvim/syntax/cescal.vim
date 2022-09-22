syn match asmType "\.long"
syn match asmType "\.ascii"
syn match asmType "\.asciz"
syn match asmType "\.byte"
syn match asmType "\.double"
syn match asmType "\.float"
syn match asmType "\.hword"
syn match asmType "\.int"
syn match asmType "\.octa"
syn match asmType "\.quad"
syn match asmType "\.short"
syn match asmType "\.single"
syn match asmType "\.space"
syn match asmType "\.string"
syn match asmType "\.word"

syn match asmIdentifier		"[a-zA-Z_][a-zA-Z0-9_]*"
syn match asmLabel		"[a-zA-Z_][a-zA-Z0-9_]*:"he=e-1
syn match inlineASM     "__asm\>"
syn match cescalNaked   "__naked\>"
syn match cescalFunc    "func\>"
syn match cescalPublic  "public\>"
syn match cescalU8  "u8\>"
syn match cescalU16  "u16\>"
syn match cescalU32  "u32\>"
syn match cescalU64  "u64\>"
syn match cescalNone  "none\>"

" Various #'s as defined by GAS ref manual sec 3.6.2.1
" Technically, the first asmDecimal is actually octal,
" since the value of 0-7 octal is the same as 0-7 decimal,
" I (Kevin) prefer to map it as decimal:
syn match asmDecimal		"\<0\+[1-7]\=\>"	 display
syn match asmDecimal		"\<[1-9]\d*\>"		 display
syn match asmOctal		"\<0[0-7][0-7]\+\>"	 display
syn match asmHexadecimal	"\<0[xX][0-9a-fA-F]\+\>" display
syn match asmBinary		"\<0[bB][0-1]\+\>"	 display

syn match asmFloat		"\<\d\+\.\d*\%(e[+-]\=\d\+\)\=\>" display
syn match asmFloat		"\.\d\+\%(e[+-]\=\d\+\)\=\>"	  display
syn match asmFloat		"\<\d\%(e[+-]\=\d\+\)\>"	  display
syn match asmFloat		"[+-]\=Inf\>\|\<NaN\>"		  display

syn match asmFloat		"\%(0[edfghprs]\)[+-]\=\d*\%(\.\d\+\)\%(e[+-]\=\d\+\)\="    display
syn match asmFloat		"\%(0[edfghprs]\)[+-]\=\d\+\%(\.\d\+\)\=\%(e[+-]\=\d\+\)\=" display
" Avoid fighting the hexadecimal match for unicorn-like '0x' prefixed floats
syn match asmFloat		"\%(0x\)[+-]\=\d*\%(\.\d\+\)\%(e[+-]\=\d\+\)\="		    display

" Allow all characters to be escaped (and in strings) as these vary across
" architectures [See sec 3.6.1.1 Strings]
syn match asmCharacterEscape	"\\."    contained
syn match asmCharacter		"'\\\=." contains=asmCharacterEscape

syn match asmStringEscape	"\\\_."			contained
syn match asmStringEscape	"\\\%(\o\{3}\|00[89]\)"	contained display
syn match asmStringEscape	"\\x\x\+"		contained display

syn region asmString		start="\"" end="\"" skip="\\\\\|\\\"" contains=asmStringEscape

syn keyword asmTodo		contained TODO FIXME XXX NOTE


syn match asmInclude		"\.include"
syn match asmCond		"\.if"
syn match asmCond		"\.else"
syn match asmCond		"\.endif"
syn match asmMacro		"\.macro"
syn match asmMacro		"\.endm"

" Assembler directives start with a '.' and may contain upper case (e.g.,
" .ABORT), numbers (e.g., .p2align), dash (e.g., .app-file) and underscore in
" CFI directives (e.g., .cfi_startproc). This will also match labels starting
" with '.', including the GCC auto-generated '.L' labels.
syn match asmDirective		"\.[A-Za-z][0-9A-Za-z-_]*"

syn case match



" Define the default highlighting.
" Only when an item doesn't have highlighting yet

" The default methods for highlighting.  Can be overridden later
hi link asmSection		Special
hi link asmLabel		Label
hi link asmComment		Comment
hi link asmTodo		Todo
hi link asmDirective	Statement

hi link asmInclude		Include
hi link asmCond		PreCondit
hi link asmMacro		Macro

if exists('g:asm_legacy_syntax_groups')
  hi link hexNumber		Number
  hi link decNumber		Number
  hi link octNumber		Number
  hi link binNumber		Number
  hi link asmHexadecimal	hexNumber
  hi link asmDecimal	decNumber
  hi link asmOctal		octNumber
  hi link asmBinary		binNumber
else
  hi link asmHexadecimal	Number
  hi link asmDecimal	Number
  hi link asmOctal		Number
  hi link asmBinary		Number
endif
hi link asmFloat		Float

hi link asmString		String
hi link asmStringEscape	Special
hi link asmCharacter	Character
hi link asmCharacterEscape	Special

hi link asmIdentifier	Identifier
hi link asmType		Type

hi link inlineASM Macro
hi link cescalPublic Macro
hi link cescalFunc Macro
hi link cescalNaked Macro
hi link cescalU8 Macro
hi link cescalU16 Macro
hi link cescalU32 Macro
hi link cescalU64 Macro
hi link cescalNone Macro
