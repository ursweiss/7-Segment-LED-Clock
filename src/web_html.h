const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LED Clock Config</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif; background: #f5f5f5; height: 100vh; overflow: hidden; }
        .app { display: flex; flex-direction: column; height: 100vh; }
        .header { background: #2c3e50; color: white; padding: 20px; }
        .header h1 { font-size: 24px; margin-bottom: 5px; }
        .header p { opacity: 0.8; font-size: 14px; }
        .main { display: flex; flex: 1; overflow: hidden; }
        .sidebar { width: 250px; background: white; border-right: 1px solid #ddd; overflow-y: auto; }
        .sidebar-menu { list-style: none; }
        .sidebar-menu li { position: relative; }
        .sidebar-menu a { display: block; padding: 15px 20px; color: #2c3e50; text-decoration: none; border-left: 3px solid transparent; transition: all 0.2s; }
        .sidebar-menu a:hover { background: #f8f9fa; }
        .sidebar-menu a.active { background: #e3f2fd; border-left-color: #3498db; font-weight: 600; }
        .badge { position: absolute; right: 15px; top: 50%; transform: translateY(-50%); min-width: 20px; height: 20px; border-radius: 10px; font-size: 11px; font-weight: 600; display: flex; align-items: center; justify-content: center; color: white; padding: 0 6px; }
        .badge.changes { background: #ff9800; }
        .badge.errors { background: #e74c3c; }
        .content { flex: 1; overflow-y: auto; padding: 30px; background: #f5f5f5; }
        .tab-panel { display: none; background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .tab-panel.active { display: block; }
        .group-title { font-size: 20px; font-weight: 600; color: #2c3e50; margin-bottom: 25px; padding-bottom: 10px; border-bottom: 2px solid #3498db; }
        .field { margin-bottom: 20px; position: relative; }
        .field label { display: block; font-weight: 500; margin-bottom: 5px; color: #34495e; }
        .field input, .field select { width: 100%; padding: 10px; padding-right: 35px; border: 1px solid #ddd; border-radius: 4px; font-size: 14px; transition: border-color 0.2s; }
        .field input.valid { border-color: #27ae60; }
        .field input.invalid { border-color: #e74c3c; }
        .field input[type="checkbox"] { width: auto; margin-right: 8px; padding-right: 10px; }
        .field input[type="color"] { height: 40px; }
        .field-help { font-size: 12px; color: #7f8c8d; margin-top: 4px; }
        .field-icon { position: absolute; right: 10px; top: 32px; font-size: 18px; font-weight: bold; }
        .field-icon.valid { color: #27ae60; }
        .field-icon.invalid { color: #e74c3c; }
        .field-error { font-size: 12px; color: #e74c3c; margin-top: 4px; }
        .footer { background: white; padding: 15px 30px; border-top: 1px solid #ddd; box-shadow: 0 -2px 4px rgba(0,0,0,0.05); display: flex; justify-content: space-between; align-items: center; }
        .actions { display: flex; gap: 10px; }
        .btn { padding: 12px 24px; border: none; border-radius: 4px; font-size: 14px; font-weight: 500; cursor: pointer; transition: background 0.2s; }
        .btn-primary { background: #3498db; color: white; }
        .btn-primary:hover { background: #2980b9; }
        .btn-secondary { background: #95a5a6; color: white; }
        .btn-secondary:hover { background: #7f8c8d; }
        .btn-danger { background: #e74c3c; color: white; }
        .btn-danger:hover { background: #c0392b; }
        .message { padding: 12px; border-radius: 4px; display: none; }
        .message.success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .message.error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .hidden { display: none !important; }
        @media (max-width: 768px) {
            .main { flex-direction: column; }
            .sidebar { width: 100%; border-right: none; border-bottom: 1px solid #ddd; max-height: 60px; }
            .sidebar-menu { display: flex; overflow-x: auto; }
            .sidebar-menu a { white-space: nowrap; border-left: none; border-bottom: 3px solid transparent; }
            .sidebar-menu a.active { border-left: none; border-bottom-color: #3498db; }
        }
    </style>
</head>
<body>
    <div class="app">
        <div class="header">
            <h1>LED Clock Configuration</h1>
            <p>Configure your 7-segment LED clock settings</p>
        </div>
        <div class="main">
            <nav class="sidebar">
                <ul class="sidebar-menu" id="sidebarMenu"></ul>
            </nav>
            <div class="content">
                <div id="message" class="message"></div>
                <form id="configForm">
                    <div id="tabPanels"></div>
                </form>
            </div>
        </div>
        <div class="footer">
            <div id="footerMessage"></div>
            <div class="actions">
                <button type="button" class="btn btn-primary" onclick="saveConfig()">Save Configuration</button>
                <button type="button" class="btn btn-secondary" onclick="loadConfig()">Reload Config</button>
                <button type="button" class="btn btn-danger" onclick="restart()">Restart Device</button>
            </div>
        </div>
    </div>
    <script>
        const TAB_ORDER = ['clock', 'led', 'weather', 'advanced', 'wifi'];
        let schema = {};
        let config = {};
        let originalConfig = {};
        let activeTab = 'clock';
        let requiresRestart = false;
        let tabStates = {};
        let fieldToTab = {};
        
        function showMessage(text, type) {
            const msg = document.getElementById('message');
            msg.textContent = text;
            msg.className = 'message ' + type;
            msg.style.display = 'block';
            setTimeout(() => msg.style.display = 'none', 5000);
        }
        
        function updateFieldVisibility() {
            schema.groups.forEach(group => {
                group.fields.forEach(field => {
                    if (field.showIf) {
                        const fieldDiv = document.getElementById('field-' + field.id);
                        if (fieldDiv) {
                            const controlInput = document.querySelector('[name="' + field.showIf.field + '"]');
                            if (controlInput) {
                                let controlValue;
                                if (controlInput.type === 'checkbox') {
                                    controlValue = controlInput.checked ? 1 : 0;
                                } else if (controlInput.tagName === 'SELECT' || controlInput.type === 'number') {
                                    controlValue = parseInt(controlInput.value);
                                } else {
                                    controlValue = controlInput.value;
                                }
                                
                                if (controlValue == field.showIf.equals) {
                                    fieldDiv.classList.remove('hidden');
                                } else {
                                    fieldDiv.classList.add('hidden');
                                }
                            }
                        }
                    }
                });
            });
        }
        
        function showTab(tabId) {
            // Hide all tabs
            document.querySelectorAll('.tab-panel').forEach(panel => {
                panel.classList.remove('active');
            });
            // Show selected tab
            const panel = document.getElementById('tab-' + tabId);
            if (panel) panel.classList.add('active');
            
            // Update menu active state
            document.querySelectorAll('.sidebar-menu a').forEach(link => {
                link.classList.remove('active');
            });
            const activeLink = document.querySelector('[data-tab="' + tabId + '"]');
            if (activeLink) activeLink.classList.add('active');
            
            activeTab = tabId;
            localStorage.setItem('activeTab', tabId);
        }
        
        function updateTabIndicators() {
            Object.keys(tabStates).forEach(tabId => {
                const state = tabStates[tabId];
                const menuItem = document.querySelector('[data-tab="' + tabId + '"]').parentElement;
                
                // Remove existing badges
                const existingBadge = menuItem.querySelector('.badge');
                if (existingBadge) existingBadge.remove();
                
                // Add error badge (priority)
                if (state.hasErrors && state.errorCount > 0) {
                    const badge = document.createElement('span');
                    badge.className = 'badge errors';
                    badge.textContent = state.errorCount;
                    menuItem.appendChild(badge);
                }
                // Add changes badge with count
                else if (state.hasChanges && state.changeCount > 0) {
                    const badge = document.createElement('span');
                    badge.className = 'badge changes';
                    badge.textContent = state.changeCount;
                    menuItem.appendChild(badge);
                }
            });
        }
        
        function checkRestartRequired() {
            const formData = new FormData(document.getElementById('configForm'));
            requiresRestart = false;
            
            for (let [key, value] of formData.entries()) {
                const field = schema.groups.flatMap(g => g.fields).find(f => f.id === key);
                if (field && field.applyMethod === 'restart') {
                    const currentValue = config[key];
                    let formValue = value;
                    
                    if (field.type === 'number') formValue = parseInt(value);
                    else if (field.type === 'checkbox') formValue = 1;
                    else if (field.type === 'color') formValue = parseInt(value.substring(1), 16);
                    
                    if (currentValue != formValue) {
                        requiresRestart = true;
                        break;
                    }
                }
            }
            
            // Check unchecked checkboxes
            schema.groups.forEach(group => {
                group.fields.forEach(field => {
                    if (field.type === 'checkbox' && !formData.has(field.id) && field.applyMethod === 'restart') {
                        if (config[field.id] != 0) {
                            requiresRestart = true;
                        }
                    }
                });
            });
            
            const submitBtn = document.querySelector('.btn-primary');
            submitBtn.textContent = requiresRestart ? 'Save Configuration & Restart' : 'Save Configuration';
        }
        
        function validateField(field, value) {
            const errors = [];
            const validation = field.validation || {};
            
            if (validation.required && (!value || value.toString().trim() === '')) {
                errors.push('This field is required');
            }
            if (validation.minLength && value.length < validation.minLength) {
                errors.push('Minimum length is ' + validation.minLength);
            }
            if (validation.maxLength && value.length > validation.maxLength) {
                errors.push('Maximum length is ' + validation.maxLength);
            }
            if (validation.min !== undefined && parseFloat(value) < validation.min) {
                errors.push('Minimum value is ' + validation.min);
            }
            if (validation.max !== undefined && parseFloat(value) > validation.max) {
                errors.push('Maximum value is ' + validation.max);
            }
            if (validation.pattern) {
                // Unescape pattern if needed
                let pattern = validation.pattern.replace(/\\\\/g, '\\');
                if (!new RegExp(pattern).test(value)) {
                    errors.push('Invalid format');
                }
            }
            
            return errors;
        }
        
        function updateFieldValidation(fieldId, isValid, errors = []) {
            const fieldDiv = document.getElementById('field-' + fieldId);
            if (!fieldDiv) return;
            
            const input = fieldDiv.querySelector('input, select');
            const existingIcon = fieldDiv.querySelector('.field-icon');
            const existingError = fieldDiv.querySelector('.field-error');
            
            // Remove existing feedback
            if (existingIcon) existingIcon.remove();
            if (existingError) existingError.remove();
            input.classList.remove('valid', 'invalid');
            
            // Add new feedback
            if (isValid) {
                input.classList.add('valid');
                const icon = document.createElement('span');
                icon.className = 'field-icon valid';
                icon.textContent = '✓';
                fieldDiv.appendChild(icon);
            } else if (errors.length > 0) {
                input.classList.add('invalid');
                const icon = document.createElement('span');
                icon.className = 'field-icon invalid';
                icon.textContent = '✗';
                fieldDiv.appendChild(icon);
                
                const errorDiv = document.createElement('div');
                errorDiv.className = 'field-error';
                errorDiv.textContent = errors[0];
                fieldDiv.appendChild(errorDiv);
            }
            
            // Update tab state
            const tabId = fieldToTab[fieldId];
            if (tabId && tabStates[tabId]) {
                // Count errors in this tab
                let errorCount = 0;
                schema.groups.find(g => g.id === tabId).fields.forEach(f => {
                    const fDiv = document.getElementById('field-' + f.id);
                    if (fDiv && fDiv.querySelector('.field-error')) errorCount++;
                });
                
                tabStates[tabId].hasErrors = errorCount > 0;
                tabStates[tabId].errorCount = errorCount;
                updateTabIndicators();
            }
        }
        
        function checkFieldChange(fieldId, value) {
            const field = schema.groups.flatMap(g => g.fields).find(f => f.id === fieldId);
            if (!field) return;
            
            const tabId = fieldToTab[fieldId];
            
            if (tabId && tabStates[tabId]) {
                // Count changed fields in this tab
                let changeCount = 0;
                schema.groups.find(g => g.id === tabId).fields.forEach(f => {
                    const fInput = document.querySelector('[name="' + f.id + '"]');
                    if (fInput) {
                        // Skip hidden fields (based on showIf conditions)
                        const fieldDiv = document.getElementById('field-' + f.id);
                        if (fieldDiv && fieldDiv.classList.contains('hidden')) {
                            return; // Skip this field
                        }
                        
                        let fValue = fInput.type === 'checkbox' ? (fInput.checked ? 1 : 0) : fInput.value;
                        let fOriginal = originalConfig[f.id];
                        
                        // Normalize values for comparison
                        if (f.type === 'number') {
                            fValue = parseInt(fValue);
                            fOriginal = parseInt(fOriginal);
                        } else if (f.type === 'color') {
                            // Convert hex color string to number for comparison
                            fValue = fInput.value ? parseInt(fInput.value.substring(1), 16) : 0;
                            fOriginal = parseInt(fOriginal);
                        } else if (f.type === 'select') {
                            // Convert both to strings for select comparison
                            fValue = String(fValue);
                            fOriginal = String(fOriginal);
                        }
                        
                        if (fValue != fOriginal) changeCount++;
                    }
                });
                
                tabStates[tabId].hasChanges = changeCount > 0;
                tabStates[tabId].changeCount = changeCount;
                updateTabIndicators();
            }
        }
        
        async function loadSchema() {
            const response = await fetch('/api/schema');
            schema = await response.json();
            
            // Sort groups by TAB_ORDER
            schema.groups.sort((a, b) => {
                const indexA = TAB_ORDER.indexOf(a.id);
                const indexB = TAB_ORDER.indexOf(b.id);
                return (indexA === -1 ? 999 : indexA) - (indexB === -1 ? 999 : indexB);
            });
            
            // Build fieldToTab mapping and initialize tabStates
            schema.groups.forEach(group => {
                tabStates[group.id] = { hasChanges: false, hasErrors: false, errorCount: 0, changeCount: 0 };
                group.fields.forEach(field => {
                    fieldToTab[field.id] = group.id;
                });
            });
        }
        
        function validateAllFields() {
            schema.groups.forEach(group => {
                group.fields.forEach(field => {
                    const input = document.querySelector('[name="' + field.id + '"]');
                    if (input) {
                        const val = input.type === 'checkbox' ? (input.checked ? 1 : 0) : input.value;
                        const errors = validateField(field, val);
                        updateFieldValidation(field.id, errors.length === 0, errors);
                    }
                });
            });
        }
        
        async function loadConfig() {
            // Check for unsaved changes
            const hasUnsavedChanges = Object.values(tabStates).some(state => state.hasChanges);
            if (hasUnsavedChanges) {
                if (!confirm('You have unsaved changes. Reload configuration anyway?')) {
                    return;
                }
            }
            
            try {
                const response = await fetch('/api/config');
                config = await response.json();
                originalConfig = JSON.parse(JSON.stringify(config));
                renderForm();
                checkRestartRequired();
                showMessage('Configuration loaded', 'success');
            } catch (e) {
                showMessage('Failed to load configuration', 'error');
            }
        }
        
        function renderForm() {
            const menuContainer = document.getElementById('sidebarMenu');
            const panelsContainer = document.getElementById('tabPanels');
            menuContainer.innerHTML = '';
            panelsContainer.innerHTML = '';
            
            schema.groups.forEach((group, index) => {
                // Create menu item
                const menuItem = document.createElement('li');
                const menuLink = document.createElement('a');
                menuLink.href = '#';
                menuLink.textContent = group.label;
                menuLink.setAttribute('data-tab', group.id);
                menuLink.onclick = (e) => {
                    e.preventDefault();
                    showTab(group.id);
                };
                if (index === 0 || group.id === activeTab) {
                    menuLink.classList.add('active');
                }
                menuItem.appendChild(menuLink);
                menuContainer.appendChild(menuItem);
                
                // Create tab panel
                const panel = document.createElement('div');
                panel.className = 'tab-panel';
                panel.id = 'tab-' + group.id;
                if (index === 0 || group.id === activeTab) {
                    panel.classList.add('active');
                }
                
                const title = document.createElement('div');
                title.className = 'group-title';
                title.textContent = group.label;
                panel.appendChild(title);
                
                // Sort fields: checkboxes with 'Enabled' in id first, then original order
                const sortedFields = [...group.fields].sort((a, b) => {
                    const aIsEnabledCheckbox = a.type === 'checkbox' && a.id.includes('Enabled');
                    const bIsEnabledCheckbox = b.type === 'checkbox' && b.id.includes('Enabled');
                    if (aIsEnabledCheckbox && !bIsEnabledCheckbox) return -1;
                    if (!aIsEnabledCheckbox && bIsEnabledCheckbox) return 1;
                    return 0;
                });
                
                sortedFields.forEach(field => {
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
                    
                    // Add event listeners
                    const inputElement = fieldDiv.querySelector('input, select');
                    if (inputElement) {
                        inputElement.addEventListener('change', function() {
                            checkRestartRequired();
                            
                            // Update field visibility based on showIf conditions
                            updateFieldVisibility();
                            
                            // Validate field
                            const val = this.type === 'checkbox' ? (this.checked ? 1 : 0) : this.value;
                            const errors = validateField(field, val);
                            updateFieldValidation(field.id, errors.length === 0, errors);
                            
                            // Check for changes
                            checkFieldChange(field.id, val);
                        });
                        
                        inputElement.addEventListener('blur', function() {
                            const val = this.type === 'checkbox' ? (this.checked ? 1 : 0) : this.value;
                            const errors = validateField(field, val);
                            updateFieldValidation(field.id, errors.length === 0, errors);
                        });
                    }
                    
                    panel.appendChild(fieldDiv);
                });
                
                panelsContainer.appendChild(panel);
            });
            
            // Restore active tab from localStorage
            const savedTab = localStorage.getItem('activeTab');
            if (savedTab && schema.groups.find(g => g.id === savedTab)) {
                showTab(savedTab);
            }
            
            // Update field visibility based on showIf conditions
            updateFieldVisibility();
            
            // Validate all fields on initial load
            validateAllFields();
        }
        
        async function saveConfig() {
            // Check for validation errors
            const hasErrors = Object.values(tabStates).some(state => state.hasErrors);
            if (hasErrors) {
                const tabsWithErrors = Object.keys(tabStates)
                    .filter(tabId => tabStates[tabId].hasErrors)
                    .map(tabId => schema.groups.find(g => g.id === tabId).label);
                alert('Please fix errors in: ' + tabsWithErrors.join(', '));
                return;
            }
            
            const formData = new FormData(document.getElementById('configForm'));
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
                    originalConfig = JSON.parse(JSON.stringify(data));
                    
                    // Reset all tab states
                    Object.keys(tabStates).forEach(tabId => {
                        tabStates[tabId].hasChanges = false;
                        tabStates[tabId].hasErrors = false;
                        tabStates[tabId].errorCount = 0;
                        tabStates[tabId].changeCount = 0;
                    });
                    updateTabIndicators();
                    
                    // Clear all validation icons
                    document.querySelectorAll('.field-icon, .field-error').forEach(el => el.remove());
                    document.querySelectorAll('input, select').forEach(el => el.classList.remove('valid', 'invalid'));
                    
                    if (requiresRestart) {
                        showMessage('Configuration saved. Restarting device...', 'success');
                        setTimeout(async () => {
                            await fetch('/api/restart', {method: 'POST'});
                            setTimeout(() => location.reload(), 10000);
                        }, 1000);
                    }
                } else {
                    showMessage('Failed to save: ' + (result.error || 'Unknown error'), 'error');
                }
            } catch (e) {
                showMessage('Failed to save configuration', 'error');
            }
        }
        
        async function restart() {
            // Check for unsaved changes
            const hasUnsavedChanges = Object.values(tabStates).some(state => state.hasChanges);
            if (hasUnsavedChanges) {
                if (!confirm('You have unsaved changes. Restart anyway?')) {
                    return;
                }
            }
            
            if (!confirm('Restart the device? This will take about 10 seconds.')) return;
            try {
                await fetch('/api/restart', {method: 'POST'});
                showMessage('Device is restarting...', 'success');
                setTimeout(() => location.reload(), 10000);
            } catch (e) {
                showMessage('Restart initiated', 'success');
            }
        }
        
        // Warn on page reload if there are unsaved changes
        window.addEventListener('beforeunload', (e) => {
            const hasUnsavedChanges = Object.values(tabStates).some(state => state.hasChanges);
            if (hasUnsavedChanges) {
                e.preventDefault();
                e.returnValue = '';
            }
        });
        
        (async () => {
            await loadSchema();
            await loadConfig();
        })();
    </script>
</body>
</html>
)rawliteral";
