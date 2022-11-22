
# Setup

1. `build_dependencies.bat` ausführen, am besten als Administrator. Damit sollten glfw und glm gebaut und die Umgebungsvariablen automatisch gesetzt werden.
2. EZR Project in VS Code/CLion/was auch immer öffnen und gucken ob es klappt,  OpenGL sollte automatisch gefunden werden.
3. Bitte nur bei Erfolg berichten :|

# Infos

- Einfach mal durchgucken, es gibt `Engine` für die Basisfunktionalität des Frameworks, `Objects` sollten aber ihre speziellen Features selbst verwalten (siehe `ColorfullTriangle` als Beispiel).
- Bitte smart pointer verwenden, also `make_unique/make_shared` statt "new". Dann müssen wir die Objekte nicht selbst löschen. Auch dazu gibt es überall schon Beispiele im Code.
- ImGui macht die Oberfläche, das ist ganz einfach, hat alle Elemente, die man so brauchen könnte und da gibt es auch schon 1-2 Beispiele zu im Code, sonst einfach Googeln.
- Es gibt ein System zur Verwaltung von Shadern, damit können wir dann die uniforms leichter setzen und den Überblick behalten.