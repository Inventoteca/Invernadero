const char *html = R"(<!DOCTYPE html><html>
    <head>
        <meta charset="UTF-8"/>
        <meta name="viewport" content="width=device-width, initial-scale=1"/>
        <title>Primera página</title>
        <style>
            body {color: gray; background-color: #101021; text-align: center; justify-content: center; align-items: center;}
            p {color: cyan; background-color: #524d6b; padding: 10px; width: 60%; margin-left: 20%;}
        </style>
    </head>
    <body>
        <p>Esto es una línea</p>
        <label for="slider">Valor:</label>
        <input type="range" id="slider" name="slider" min="0" max="100" value="50" style="width: 50%;">
    </body>
</html>)";
