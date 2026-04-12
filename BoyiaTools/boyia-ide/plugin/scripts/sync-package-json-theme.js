'use strict';

/**
 * Merges boyia-token-defaults.js → package.json contributes.configurationDefaults
 */

const fs = require('fs');
const path = require('path');
const { buildConfigurationDefaults } = require('../src/theme/boyia-token-defaults');

const pkgPath = path.join(__dirname, '..', 'package.json');
const raw = fs.readFileSync(pkgPath, 'utf8');
const pkg = JSON.parse(raw);

pkg.contributes = pkg.contributes || {};
pkg.contributes.configurationDefaults = buildConfigurationDefaults();

fs.writeFileSync(pkgPath, `${JSON.stringify(pkg, null, '\t')}\n`, 'utf8');
console.log('sync-package-json-theme: wrote contributes.configurationDefaults from src/theme/boyia-token-defaults.js');
