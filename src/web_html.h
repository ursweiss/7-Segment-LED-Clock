const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LED Clock Config</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif; background: #f5f5f5; padding: 20px; }
        .container { max-width: 800px; margin: 0 auto; }
        .header { background: #2c3e50; color: white; padding: 20px; border-radius: 8px 8px 0 0; }
        .header h1 { font-size: 24px; margin-bottom: 5px; }
        .header p { opacity: 0.8; font-size: 14px; }
        .content { background: white; padding: 20px; border-radius: 0 0 8px 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .group { margin-bottom: 30px; }
        .group-title { font-size: 18px; font-weight: 600; color: #2c3e50; margin-bottom: 15px; padding-bottom: 8px; border-bottom: 2px solid #3498db; }
        .field { margin-bottom: 20px; }
        .field label { display: block; font-weight: 500; margin-bottom: 5px; color: #34495e; }
        .field input, .field select { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 4px; font-size: 14px; }
        .field input[type="checkbox"] { width: auto; margin-right: 8px; }
        .field input[type="color"] { height: 40px; }
        .field-help { font-size: 12px; color: #7f8c8d; margin-top: 4px; }
        .actions { display: flex; gap: 10px; margin-top: 20px; padding-top: 20px; border-top: 1px solid #eee; }
        .btn { padding: 12px 24px; border: none; border-radius: 4px; font-size: 14px; font-weight: 500; cursor: pointer; transition: background 0.2s; }
        .btn-primary { background: #3498db; color: white; }
        .btn-primary:hover { background: #2980b9; }
        .btn-secondary { background: #95a5a6; color: white; }
        .btn-secondary:hover { background: #7f8c8d; }
        .btn-danger { background: #e74c3c; color: white; }
        .btn-danger:hover { background: #c0392b; }
        .message { padding: 12px; border-radius: 4px; margin-bottom: 15px; display: none; }
        .message.success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .message.error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .hidden { display: none !important; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>LED Clock Configuration</h1>
            <p>Configure your 7-segment LED clock settings</p>
        </div>
        <div class="content">
            <div id="message" class="message"></div>
            <form id="configForm">
                <div id="fields"></div>
                <div class="actions">
                    <button type="submit" class="btn btn-primary">Save Configuration</button>
                    <button type="button" class="btn btn-secondary" onclick="loadConfig()">Reload</button>
                    <button type="button" class="btn btn-danger" onclick="restart()">Restart Device</button>
                </div>
            </form>
        </div>
    </div>
    <script>
        let schema = {};
        let config = {};
        
        function showMessage(text, type) {
            const msg = document.getElementById('message');
            msg.textContent = text;
            msg.className = 'message ' + type;
            msg.style.display = 'block';
            setTimeout(() => msg.style.display = 'none', 5000);
        }
        
        async function loadSchema() {
            const response = await fetch('/api/schema');
            schema = await response.json();
        }
        
        async function loadConfig() {
            try {
                const response = await fetch('/api/config');
                config = await response.json();
                renderForm();
                showMessage('Configuration loaded', 'success');
            } catch (e) {
                showMessage('Failed to load configuration', 'error');
            }
        }
        
        function renderForm() {
            const container = document.getElementById('fields');
            container.innerHTML = '';
            
            schema.groups.forEach(group => {
                const groupDiv = document.createElement('div');
                groupDiv.className = 'group';
                groupDiv.innerHTML = '<div class="group-title">' + group.label + '</div>';
                
                group.fields.forEach(field => {
                    const fieldDiv = document.createElement('div');
                    fieldDiv.className = 'field';
                    fieldDiv.id = 'field-' + field.id;
                    
                    let input = '';
                    const value = config[field.id] !== undefined ? config[field.id] : field.default;
                    
                    if (field.type === 'text' || field.type === 'password' || field.type === 'time') {
                        input = '<input type="' + field.type + '" name="' + field.id + '" value="' + value + '">';
                    } else if (field.type === 'number') {
                        input = '<input type="number" name="' + field.id + '" value="' + value + '">';
                    } else if (field.type === 'color') {
                        const hexValue = '#' + ('000000' + (value >>> 0).toString(16)).slice(-6);
                        input = '<input type="color" name="' + field.id + '" value="' + hexValue + '">';
                    } else if (field.type === 'checkbox') {
                        input = '<input type="checkbox" name="' + field.id + '" ' + (value ? 'checked' : '') + '>';
                    } else if (field.type === 'select') {
                        input = '<select name="' + field.id + '">';
                        field.options.forEach(opt => {
                            input += '<option value="' + opt.value + '" ' + (value == opt.value ? 'selected' : '') + '>' + opt.label + '</option>';
                        });
                        input += '</select>';
                    }
                    
                    fieldDiv.innerHTML = '<label>' + field.label + '</label>' + input + (field.help ? '<div class="field-help">' + field.help + '</div>' : '');
                    groupDiv.appendChild(fieldDiv);
                });
                
                container.appendChild(groupDiv);
            });
        }
        
        document.getElementById('configForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            const formData = new FormData(e.target);
            const data = {};
            
            for (let [key, value] of formData.entries()) {
                const field = schema.groups.flatMap(g => g.fields).find(f => f.id === key);
                if (field) {
                    if (field.type === 'number') {
                        data[key] = parseInt(value);
                    } else if (field.type === 'checkbox') {
                        data[key] = 1;
                    } else if (field.type === 'color') {
                        data[key] = parseInt(value.substring(1), 16);
                    } else {
                        data[key] = value;
                    }
                }
            }
            
            // Handle unchecked checkboxes
            schema.groups.forEach(group => {
                group.fields.forEach(field => {
                    if (field.type === 'checkbox' && !formData.has(field.id)) {
                        data[field.id] = 0;
                    }
                });
            });
            
            try {
                const response = await fetch('/api/config', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify(data)
                });
                const result = await response.json();
                if (result.success) {
                    showMessage('Configuration saved successfully', 'success');
                    config = data;
                } else {
                    showMessage('Failed to save: ' + (result.error || 'Unknown error'), 'error');
                }
            } catch (e) {
                showMessage('Failed to save configuration', 'error');
            }
        });
        
        async function restart() {
            if (!confirm('Restart the device? This will take about 10 seconds.')) return;
            try {
                await fetch('/api/restart', {method: 'POST'});
                showMessage('Device is restarting...', 'success');
                setTimeout(() => location.reload(), 10000);
            } catch (e) {
                showMessage('Restart initiated', 'success');
            }
        }
        
        (async () => {
            await loadSchema();
            await loadConfig();
        })();
    </script>
</body>
</html>
)rawliteral";
