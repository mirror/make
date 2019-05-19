(
 (nil . ((bug-reference-bug-regexp . "\\(\\)\\bSV[- ]\\([0-9]+\\)")
         (bug-reference-url-format . "https://savannah.gnu.org/bugs/?%s")
         (ccls-initialization-options . (:index (:threads 6
                                                 :initialBlacklist ("/make-[0-9]" "tests/work/" "/\\.deps" "/\\..*cache" "/\\.git"))))
         (lsp-file-watch-ignored . ("/\\.git$"
                                    "/\\..*cache$"
                                    ;; autotools content
                                    "/\\.deps$"
                                    "/autom4te\\.cache$"
                                    "/build-aux$"
                                    ;; make-specific content
                                    "/doc/manual$"
                                    "/tests/work$"
                                    "/make-[0-9]"))
         ))
 (c-mode . ((c-file-style . "gnu")))
)
