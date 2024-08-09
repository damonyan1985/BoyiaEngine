const BaseRegister = require('./BaseRegister');

class UtilRegister extends BaseRegister {
  exec(text, config) {
    return this.getAssist(['Util']);
  }

  isConstant() {
    return true;
  }
}

module.exports = UtilRegister;