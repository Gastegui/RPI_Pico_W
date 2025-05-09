#ifndef PAGINA_H
#define PAGINA_H

const char* pagina = 
"<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "    <meta charset=\"UTF-8\">\n"
    "    <title>Pico W Button Control</title>\n"
    "    <style>\n"
    "        body { \n"
    "            display: flex; \n"
    "            justify-content: center; \n"
    "            align-items: center; \n"
    "            height: 100vh; \n"
    "            margin: 0; \n"
    "            font-family: Arial, sans-serif;\n"
    "        }\n"
    "        .button-grid {\n"
    "            display: grid;\n"
    "            grid-template-areas: \n"
    "                '. top .'\n"
    "                'left center right'\n"
    "                '. bottom .';\n"
    "            gap: 10px;\n"
    "            grid-template-columns: 100px 100px 100px;\n"
    "            grid-template-rows: 100px 100px 100px;\n"
    "        }\n"
    "        .button {\n"
    "            border: none;\n"
    "            background-color: #3498db;\n"
    "            color: white;\n"
    "            font-size: 16px;\n"
    "            cursor: pointer;\n"
    "            transition: background-color 0.3s;\n"
    "        }\n"
    "        .button:hover {\n"
    "            background-color: #2980b9;\n"
    "        }\n"
    "        #status {\n"
    "            margin-top: 20px;\n"
    "            text-align: center;\n"
    "        }\n"
    "        .top { grid-area: top; }\n"
    "        .bottom { grid-area: bottom; }\n"
    "        .left { grid-area: left; }\n"
    "        .right { grid-area: right; }\n"
    "        .center { grid-area: center; }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "    <div class=\"container\">\n"
    "        <div class=\"button-grid\">\n"
    "            <button class=\"button top\" onclick=\"sendButtonPress('forward')\">Top</button>\n"
    "            <button class=\"button bottom\" onclick=\"sendButtonPress('backward')\">Bottom</button>\n"
    "            <button class=\"button left\" onclick=\"sendButtonPress('left')\">Left</button>\n"
    "            <button class=\"button right\" onclick=\"sendButtonPress('right')\">Right</button>\n"
    "            <button class=\"button center\" onclick=\"sendButtonPress('stop')\">Center</button>\n"
    "        </div>\n"
    "        <div id=\"status\"></div>\n"
    "    </div>\n"
    "\n"
    "    <script>\n"
    "        function sendButtonPress(button) {\n"
    "            const statusElement = document.getElementById('status');\n"
    "            \n"
    "            fetch('/', {\n"
    "                method: 'POST',\n"
    "                headers: {\n"
    "                    'Content-Type': 'application/x-www-form-urlencoded',\n"
    "                },\n"
    "                body: `button=${button}`\n"
    "            })\n"
    "            .then(response => {\n"
    "                if (response.ok) {\n"
    "                    statusElement.textContent = 'OK: Button ' + button + ' pressed';\n"
    "                    statusElement.style.color = 'green';\n"
    "                } else {\n"
    "                    statusElement.textContent = 'NOT OK';\n"
    "                    statusElement.style.color = 'red';\n"
    "                }\n"
    "            })\n"
    "            .catch(error => {\n"
    "                statusElement.textContent = 'Error: ' + error;\n"
    "                statusElement.style.color = 'red';\n"
    "            });\n"
    "        }\n"
    "    </script>\n"
    "</body>\n"
    "</html>";

#endif