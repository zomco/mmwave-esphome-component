// r60abd1-dashboard-card.js
// Home Assistant Lovelace Custom Card for R60ABD1 Radar Status and Waveform Display

class R60ABD1DashboardCard extends HTMLElement {
    constructor() {
      super();
      this.attachShadow({ mode: 'open' });
      this._hass = null;
      this._config = null;
  
      const card = document.createElement('ha-card');
      this._cardElement = card;
  
      const style = document.createElement('style');
      style.textContent = `
        .content { padding: 16px; }
        .section { margin-bottom: 20px; border: 1px solid var(--divider-color); border-radius: var(--ha-card-border-radius, 4px); padding: 12px; }
        .section.hidden-section { display: none !important; } /* For hiding entire sections */
        .section-title {
          font-size: 1.2em;
          font-weight: bold;
          margin-bottom: 10px;
          color: var(--primary-text-color);
          border-bottom: 1px solid var(--divider-color);
          padding-bottom: 6px;
          display: flex;
          align-items: center;
        }
        .section-title ha-icon { margin-right: 8px; }
        .grid-container {
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
          gap: 10px;
        }
        .grid-item {
          display: flex;
          flex-direction: column;
          align-items: center;
          padding: 8px;
          background-color: var(--ha-card-background, var(--card-background-color, white));
          border-radius: var(--ha-card-border-radius, 4px);
          /* box-shadow: var(--ha-card-box-shadow, none); removed for cleaner look within section */
          text-align: center;
        }
        .grid-item ha-icon { color: var(--paper-item-icon-color); margin-bottom: 4px; }
        .item-name { font-size: 0.9em; color: var(--secondary-text-color); }
        .item-state { font-size: 1.1em; font-weight: bold; color: var(--primary-text-color); word-break: break-all; }
        .waveform-container { margin-top: 12px; }
        .error-message { color: var(--error-color); margin-top: 10px; font-size: 0.9em; }
        .warning-message { color: var(--warning-color); margin-top: 10px; font-size: 0.9em; }
        .hidden { display: none !important; } /* For hiding elements within sections */
      `;
  
      this._contentContainer = document.createElement('div');
      this._contentContainer.className = 'content';
  
      card.appendChild(style);
      card.appendChild(this._contentContainer);
      this.shadowRoot.appendChild(card);
    }
  
    setConfig(config) {
      this._config = config;
      this._cardElement.header = config.title || "雷达综合面板 (Radar Dashboard)";
  
      // Create section containers - they will be added to _contentContainer in renderCardContent
      this._presenceSection = this._createSectionElement("人在监测 (Presence & Motion)", "mdi:account-search-outline");
      this._heartRateSection = this._createSectionElement("心率监测 (Heart Rate)", "mdi:heart-pulse");
      this._respirationSection = this._createSectionElement("呼吸监测 (Respiration)", "mdi:lungs");
      this._sleepSection = this._createSectionElement("睡眠监测 (Sleep)", "mdi:bed-clock");
      this._infoSection = this._createSectionElement("设备信息 (Device Info)", "mdi:information-outline");
  
      if (this._hass) {
        this.renderCardContent();
      }
    }
  
    set hass(hass) {
      this._hass = hass;
      this.renderCardContent();
    }
  
    _createSectionElement(title, icon = 'mdi:view-dashboard-variant-outline') {
      const section = document.createElement('div');
      section.className = 'section'; // Default class
      if (title) {
        const titleEl = document.createElement('div');
        titleEl.className = 'section-title';
        const iconEl = document.createElement('ha-icon');
        iconEl.icon = icon;
        titleEl.appendChild(iconEl);
        titleEl.appendChild(document.createTextNode(title));
        section.appendChild(titleEl);
      }
      const gridContainer = document.createElement('div');
      gridContainer.className = 'grid-container';
      section.appendChild(gridContainer); // Add grid container for items
      return section;
    }
  
    _getEntityState(entityId, attribute) {
      if (!this._hass || !entityId || !this._hass.states[entityId]) return "N/A";
      const stateObj = this._hass.states[entityId];
      return attribute ? (stateObj.attributes[attribute] || "N/A") : stateObj.state;
    }
  
    _getEntityStateObj(entityId) {
      return (this._hass && entityId && this._hass.states[entityId]) ? this._hass.states[entityId] : null;
    }
  
    _isSwitchOn(entityId) {
      const stateObj = this._getEntityStateObj(entityId);
      return stateObj ? stateObj.state === 'on' : false; // Default to false if no control entity
    }
  
    _renderGridItem(container, name, entityId, icon, unit = "", options = {}) {
      if (!entityId && !options.literal_value) return; // Need either entity or literal value
  
      const item = document.createElement('div');
      item.className = 'grid-item';
      const state = options.literal_value !== undefined ? options.literal_value : this._getEntityState(entityId);
      const nameToShow = name || (this._getEntityStateObj(entityId)?.attributes?.friendly_name || entityId || "N/A");
  
      item.innerHTML = `
        <ha-icon icon="${icon || 'mdi:eye-outline'}"></ha-icon>
        <div class="item-name">${nameToShow}</div>
        <div class="item-state">${state} ${unit}</div>
      `;
      container.querySelector('.grid-container').appendChild(item);
    }
  
    renderCardContent() {
      if (!this._hass || !this._config) {
        this._contentContainer.innerHTML = '<p class="warning-message">等待配置或HASS对象...</p>';
        return;
      }
      const { entities = {}, controls = {}, waveform_options = {} } = this._config;
  
      // Clear previous content and re-add sections to ensure order and clean slate
      while (this._contentContainer.firstChild) {
          this._contentContainer.removeChild(this._contentContainer.firstChild);
      }
      this._contentContainer.appendChild(this._presenceSection);
      this._contentContainer.appendChild(this._heartRateSection);
      this._contentContainer.appendChild(this._respirationSection);
      this._contentContainer.appendChild(this._sleepSection);
      this._contentContainer.appendChild(this._infoSection);
  
  
      // --- 1. 人在监测 (Presence & Motion) ---
      const showPresenceSection = controls.presence_detection_switch ? this._isSwitchOn(controls.presence_detection_switch) : true;
      this._presenceSection.classList.toggle('hidden-section', !showPresenceSection);
      if (showPresenceSection) {
        this._presenceSection.querySelector('.grid-container').innerHTML = ''; // Clear previous items
        this._renderGridItem(this._presenceSection, "人体存在", entities.presence, "mdi:account-check-outline");
        this._renderGridItem(this._presenceSection, "运动状态", entities.motion_text, "mdi:walk");
        this._renderGridItem(this._presenceSection, "目标距离", entities.distance, "mdi:map-marker-distance", "cm");
        this._renderGridItem(this._presenceSection, "位置X", entities.position_x, "mdi:axis-x-arrow", "cm");
        this._renderGridItem(this._presenceSection, "位置Y", entities.position_y, "mdi:axis-y-arrow", "cm");
        this._renderGridItem(this._presenceSection, "位置Z", entities.position_z, "mdi:axis-z-arrow", "cm");
      }
  
      // --- 2. 心率监测 (Heart Rate) ---
      const showHRSection = controls.heart_rate_detection_switch ? this._isSwitchOn(controls.heart_rate_detection_switch) : true;
      this._heartRateSection.classList.toggle('hidden-section', !showHRSection);
      if (showHRSection) {
        this._heartRateSection.querySelector('.grid-container').innerHTML = ''; // Clear previous items
        this._renderGridItem(this._heartRateSection, "心率值", entities.heart_rate, "mdi:heart-flash", "bpm");
  
        const showHRWave = controls.heart_rate_waveform_reporting_switch ? this._isSwitchOn(controls.heart_rate_waveform_reporting_switch) : true;
        let hrWaveContainer = this._heartRateSection.querySelector('.waveform-container.hr-wave');
        if (showHRWave && entities.heart_rate_wave) {
          if (!hrWaveContainer) {
            hrWaveContainer = document.createElement('div');
            hrWaveContainer.className = 'waveform-container hr-wave'; // Add specific class
            this._heartRateSection.appendChild(hrWaveContainer); // Append after grid
          }
          const hrOptions = {
              ...(waveform_options.default_waveform_options || {}),
              ...(waveform_options.heart_rate || {}),
              yAxisLabel: "振幅 (mm)"
          };
          this._renderWaveformToSVG(hrWaveContainer, entities.heart_rate_wave, hrOptions, this.convertHeartRateWaveValue.bind(this));
        } else if (hrWaveContainer) {
          hrWaveContainer.remove(); // Remove if switch is off or no entities
        }
      }
  
      // --- 3. 呼吸监测 (Respiration) ---
      const showRespSection = controls.respiration_detection_switch ? this._isSwitchOn(controls.respiration_detection_switch) : true;
      this._respirationSection.classList.toggle('hidden-section', !showRespSection);
      if (showRespSection) {
        this._respirationSection.querySelector('.grid-container').innerHTML = ''; // Clear previous items
        this._renderGridItem(this._respirationSection, "呼吸率", entities.respiration_rate, "mdi:chart-gantt", "rpm");
        this._renderGridItem(this._respirationSection, "呼吸状态", entities.respiration_info, "mdi:information-outline");
  
        const showRespWave = controls.respiration_waveform_reporting_switch ? this._isSwitchOn(controls.respiration_waveform_reporting_switch) : true;
        let respWaveContainer = this._respirationSection.querySelector('.waveform-container.resp-wave');
        if (showRespWave && entities.respiration_wave) {
          if (!respWaveContainer) {
            respWaveContainer = document.createElement('div');
            respWaveContainer.className = 'waveform-container resp-wave'; // Add specific class
            this._respirationSection.appendChild(respWaveContainer); // Append after grid
          }
          const respOptions = {
              ...(waveform_options.default_waveform_options || {}),
              ...(waveform_options.respiration || {}),
              yAxisLabel: "呼吸振幅 (单位)"
          };
          this._renderWaveformToSVG(respWaveContainer, entities.respiration_wave, respOptions, this.convertRespirationWaveValue.bind(this));
        } else if (respWaveContainer) {
          respWaveContainer.remove(); // Remove if switch is off or no entities
        }
      }
  
      // --- 4. 睡眠监测 (Sleep) ---
      const showSleepSection = controls.sleep_monitoring_switch ? this._isSwitchOn(controls.sleep_monitoring_switch) : true;
      this._sleepSection.classList.toggle('hidden-section', !showSleepSection);
      if (showSleepSection) {
        this._sleepSection.querySelector('.grid-container').innerHTML = ''; // Clear previous items
        this._renderGridItem(this._sleepSection, "在床状态", entities.bed_status, "mdi:bed-outline");
        this._renderGridItem(this._sleepSection, "睡眠阶段", entities.sleep_stage, "mdi:sleep");
        this._renderGridItem(this._sleepSection, "睡眠评分", entities.sleep_score, "mdi:star-check-outline", "分");
        this._renderGridItem(this._sleepSection, "体动幅度", entities.body_movement, "mdi:human-male-height", "%");
        // Add other sleep entities here if configured: struggle_status, unattended_status etc.
      }
  
      // --- 5. 设备信息 (Device Info) ---
      // This section is always shown if entities are provided, not tied to a specific functional switch in this example
      this._infoSection.querySelector('.grid-container').innerHTML = ''; // Clear previous items
      let infoHasContent = false;
      if (entities.firmware_version) {
          this._renderGridItem(this._infoSection, "固件版本", entities.firmware_version, "mdi:chip");
          infoHasContent = true;
      }
      // You can add more device-specific info here if needed
      this._infoSection.classList.toggle('hidden-section', !infoHasContent);
    }
  
    convertHeartRateWaveValue(rawValue) {
      if (rawValue === null || rawValue === undefined) return null;
      const calculated_amplitude = (rawValue - 128) * 0.015;
      // Offset based on the sign of calculated_amplitude (which is same as sign of rawValue - 128)
      let offset = 0;
      if (rawValue > 128) offset = 0.1;
      else if (rawValue < 128) offset = -0.1;
      return calculated_amplitude + offset;
    }
  
    convertRespirationWaveValue(rawValue) {
      if (rawValue === null || rawValue === undefined) return null;
      // For respiration, we plot (rawValue - 128) directly as per current understanding
      // The Y-axis will represent these "amplitude units"
      return rawValue - 128;
    }
  
    _renderWaveformToSVG(container, waveformEntityIds, options = {}, conversionFunction) {
      if (!waveformEntityIds || !Array.isArray(waveformEntityIds) || waveformEntityIds.length !== 5) {
        container.innerHTML = '<p class="error-message">波形图实体配置错误。</p>';
        return;
      }
  
      const waveformValuesRaw = waveformEntityIds.map(entityId => {
        const stateObj = this._getEntityStateObj(entityId);
        return (stateObj && !isNaN(parseFloat(stateObj.state))) ? parseFloat(stateObj.state) : null;
      });
  
      const waveformValuesConverted = waveformValuesRaw.map(val => conversionFunction(val));
  
      let hasNullData = waveformValuesConverted.some(val => val === null);
      let allNullData = waveformValuesConverted.every(val => val === null);
  
      // Manage warning message for partial data
      let warningMsgElement = container.querySelector('.warning-message.partial-data');
      if (hasNullData && !allNullData) {
          if (!warningMsgElement) {
              warningMsgElement = document.createElement('p');
              warningMsgElement.className = 'warning-message partial-data';
              warningMsgElement.textContent = '部分波形数据点无效，图形可能不完整。';
              container.insertBefore(warningMsgElement, container.firstChild);
          }
      } else if (warningMsgElement) {
          warningMsgElement.remove();
      }
      
      if (allNullData && waveformEntityIds.every(id => !this._getEntityStateObj(id))) {
          container.innerHTML = '<p class="warning-message">等待波形数据...</p>';
          return;
      }
      if (allNullData) {
          // If all are null but entities exist, keep the warning or show "No data"
          // For now, let it fall through to potentially draw an empty graph if desired, or clear
          // container.innerHTML = '<p class="warning-message">所有波形数据点均无效。</p>';
          // return; // Or clear the SVG
      }
  
  
      const svgWidth = options.svg_width || 300;
      const svgHeight = options.svg_height || 100;
      const paddingTop = options.padding_top || 15;
      const paddingBottom = options.padding_bottom || 25;
      const paddingLeft = options.padding_left || 35;
      const paddingRight = options.padding_right || 15;
      const graphWidth = svgWidth - paddingLeft - paddingRight;
      const graphHeight = svgHeight - paddingTop - paddingBottom;
  
      // Determine Y-axis range dynamically if not set, or use defaults/options
      let yMin, yMax;
      if (options.y_min === undefined || options.y_max === undefined) {
          const validValues = waveformValuesConverted.filter(v => v !== null);
          if (validValues.length > 0) {
              const dataMin = Math.min(...validValues);
              const dataMax = Math.max(...validValues);
              const dataRange = dataMax - dataMin;
              yMin = options.y_min !== undefined ? options.y_min : Math.floor(dataMin - dataRange * 0.1); // Add some padding
              yMax = options.y_max !== undefined ? options.y_max : Math.ceil(dataMax + dataRange * 0.1);
              if (yMin === yMax) { // Avoid division by zero if all points are same
                  yMin -= 1;
                  yMax += 1;
              }
          } else { // No valid data points
              yMin = options.y_min !== undefined ? options.y_min : -1;
              yMax = options.y_max !== undefined ? options.y_max : 1;
          }
      } else {
          yMin = options.y_min;
          yMax = options.y_max;
      }
      const yRange = yMax - yMin;
  
      if (yRange <= 0) {
          container.innerHTML = '<p class="error-message">Y轴范围配置错误或无有效数据以确定范围。</p>';
          return;
      }
  
      const numPoints = 5;
      const xStep = graphWidth / (numPoints > 1 ? numPoints - 1 : 1);
  
      // Find or create SVG element
      let svgEl = container.querySelector('svg');
      if (!svgEl) {
          svgEl = document.createElementNS("http://www.w3.org/2000/svg", "svg");
          container.appendChild(svgEl);
      }
      svgEl.setAttribute('width', svgWidth);
      svgEl.setAttribute('height', svgHeight);
      svgEl.style.border = "1px solid var(--divider-color)";
      svgEl.style.borderRadius = "4px";
      
      let svgContent = `
          <style>
            .axis { font-size: 9px; fill: var(--secondary-text-color); }
            .grid-line { stroke: var(--divider-color); stroke-opacity: 0.5; stroke-dasharray: 2,2; }
            .waveform-line { stroke: ${options.line_color || 'var(--accent-color)'}; stroke-width: ${options.stroke_width || 1.5}; fill: none; }
            .data-point { fill: ${options.line_color || 'var(--accent-color)'}; }
            .axis-label { font-size: 10px; fill: var(--secondary-text-color); text-anchor: middle; }
          </style>
      `;
  
      // Y-axis label
      svgContent += `<text x="${paddingLeft / 3}" y="${paddingTop + graphHeight / 2}" class="axis-label" transform="rotate(-90, ${paddingLeft/3}, ${paddingTop + graphHeight/2})">${options.yAxisLabel || 'Amplitude'}</text>`;
  
      // Y-axis and grid lines
      const numYGridLines = options.y_grid_lines || 3;
      for (let i = 0; i <= numYGridLines; i++) {
        const y = paddingTop + (i * graphHeight / numYGridLines);
        const yValue = yMax - (i * yRange / numYGridLines);
        svgContent += `<line x1="${paddingLeft}" y1="${y}" x2="${svgWidth - paddingRight}" y2="${y}" class="grid-line" />`;
        svgContent += `<text x="${paddingLeft - 5}" y="${y + 3}" text-anchor="end" class="axis">${yValue.toFixed(options.y_axis_decimals === undefined ? 2 : options.y_axis_decimals)}</text>`;
      }
  
      // X-axis and labels
      svgContent += `<line x1="${paddingLeft}" y1="${paddingTop + graphHeight}" x2="${svgWidth - paddingRight}" y2="${paddingTop + graphHeight}" class="grid-line" />`; // X-axis line
      for (let i = 0; i < numPoints; i++) {
        const x = paddingLeft + (i * xStep);
        svgContent += `<text x="${x}" y="${svgHeight - paddingBottom + 12}" text-anchor="middle" class="axis">${(i * (options.time_step || 0.2)).toFixed(1)}s</text>`;
      }
  
      let pathData = "";
      let firstValidPointProcessed = false;
      waveformValuesConverted.forEach((value, index) => {
        if (value === null) return;
        const x = paddingLeft + (index * xStep);
        const y = paddingTop + graphHeight - ((Math.max(yMin, Math.min(yMax, value)) - yMin) / yRange * graphHeight);
  
        pathData += firstValidPointProcessed ? ` L${x},${y}` : `M${x},${y}`;
        firstValidPointProcessed = true;
        if (options.show_points) {
          svgContent += `<circle cx="${x}" cy="${y}" r="${options.point_radius || 2}" class="data-point" />`;
        }
      });
  
      if (firstValidPointProcessed) {
          svgContent += `<path d="${pathData}" class="waveform-line" />`;
      }
      svgContent += `</svg>`;
      svgEl.innerHTML = svgContent;
    }
  
    getCardSize() {
      let size = 1; // Base for title
      if (!this._presenceSection.classList.contains('hidden-section')) size += 2;
      if (!this._heartRateSection.classList.contains('hidden-section')) size += (this._config.entities?.heart_rate_wave && this._isSwitchOn(this._config.controls?.heart_rate_waveform_reporting_switch) ? 2 : 1);
      if (!this._respirationSection.classList.contains('hidden-section')) size += (this._config.entities?.respiration_wave && this._isSwitchOn(this._config.controls?.respiration_waveform_reporting_switch) ? 2 : 1);
      if (!this._sleepSection.classList.contains('hidden-section')) size += 2;
      if (!this._infoSection.classList.contains('hidden-section')) size += 1;
      return Math.max(1, Math.ceil(size / 1.5)); // Adjusted rough estimate
    }
  
    static getStubConfig() {
      return {
        title: "雷达综合面板 (预览)",
        entities: {
          presence: "binary_sensor.placeholder_presence",
          motion_text: "text_sensor.placeholder_motion",
          bed_status: "binary_sensor.placeholder_bed",
          distance: "sensor.placeholder_distance",
          heart_rate: "sensor.placeholder_hr",
          heart_rate_wave: ["sensor.hrw1", "sensor.hrw2", "sensor.hrw3", "sensor.hrw4", "sensor.hrw5"],
          respiration_rate: "sensor.placeholder_resp_rate",
          respiration_info: "text_sensor.placeholder_resp_info",
          respiration_wave: ["sensor.respw1", "sensor.respw2", "sensor.respw3", "sensor.respw4", "sensor.respw5"],
          sleep_stage: "text_sensor.placeholder_sleep_stage",
          sleep_score: "sensor.placeholder_sleep_score",
          body_movement: "sensor.placeholder_body_mvmt",
          firmware_version: "text_sensor.placeholder_fw",
        },
        controls: {
          presence_detection_switch: "switch.placeholder_presence_ctrl",
          heart_rate_detection_switch: "switch.placeholder_hr_ctrl",
          heart_rate_waveform_reporting_switch: "switch.placeholder_hr_wave_ctrl",
          respiration_detection_switch: "switch.placeholder_resp_ctrl",
          respiration_waveform_reporting_switch: "switch.placeholder_resp_wave_ctrl",
          sleep_monitoring_switch: "switch.placeholder_sleep_ctrl",
        },
        waveform_options: {
          default_waveform_options: { svg_height: 120, y_axis_decimals: 1 },
          heart_rate: { line_color: "tomato" },
          respiration: { line_color: "steelblue", y_axis_decimals: 0 },
        }
      };
    }
  }
  
  customElements.define('r60abd1-dashboard-card', R60ABD1DashboardCard);
  window.customCards = window.customCards || [];
  window.customCards.push({
    type: "r60abd1-dashboard-card",
    name: "R60ABD1 雷达综合面板",
    description: "显示R60ABD1雷达的多种状态和波形数据，并根据功能开关控制各区域显示。",
    preview: true,
  });
  