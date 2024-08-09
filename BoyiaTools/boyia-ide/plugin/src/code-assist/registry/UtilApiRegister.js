const BaseRegister = require('./BaseRegister');

class UtilApiRegister extends BaseRegister {
  exec(text, config) {
    if (text === 'Util.') {
      return this.getAssist(config.util);
    }

    return this.getAssist([]);
  }

  isApi() {
    return true;
  }
}

module.exports = UtilApiRegister;