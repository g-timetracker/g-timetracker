TEMPLATE = subdirs

SUBDIRS += \
    lib \
    app

app.depends = lib

TRANSLATIONS =  app/translations/timetracker_en.ts \
                app/translations/timetracker_ru.ts
