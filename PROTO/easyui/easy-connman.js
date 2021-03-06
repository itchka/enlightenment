/**
 * A simple client to connman. Provides an interface to see available services,
 * technologies and do simple adjusts to them. Uses elev8 dbus module to
 * interact with connman.
 * As any easyUI app, is basically made of models and controllers. Models
 * communicate with connman via dbus, and controllers just display, in a pretty
 * way, models data. Usually, models register listeners to keep up-to-date with
 * connman, providing a "real time" interface. Models also do the "heavy task"
 * of organising the data in a way that simplifies controllers task, so, usually,
 * easy-connman controllers are realy simple (but not exactly the models).
 */

var dbus = require('dbus');
var EUI = require('eui');

var conn = new dbus.Connection('system');
var obj = conn.getObject('net.connman', '/');
var manager = obj.getProxy('net.connman.Manager');
var image_dir = './themes/easy-connman/images/';

/**
 * Converts the DBus dictionary to javascript dictonary. DBus dictionary
 * is actually an array of the form [["key1", [value1]], ["key2", [value2]]],
 * where //valueN// can be any DBus type, even array or other dictionary.
 * As the underlying DBus module on elev8 doesn't convert it to us, we
 * need to do so.
 */
var arrayDictToDict = function(array){
  var dict = {};
  for (var i in array) {
    var key = array[i][0];
    var value = array[i][1];

    if ((typeof(key) == 'string') && (value instanceof Array)) {
      if (value[0] instanceof Array)
        dict[key] = arrayDictToDict(value[0]);
      else
        dict[key] = value[0];
    } else
      return array;
  }

  return dict;
};

/**
 * Join the elements of an array with commas, as used to some
 * connman properties.
 */
var joinArray = function(array) {
  if (array.length)
    return array.join(', ');

  return '';
};

/**
 * Simple error handler. As all DBus errors have //name// and //message//,
 * it's a helper to display then. Can easyly be modified in future to
 * display, for example, a UI message to the user.
 */
var genericErrorHandler = function(name, message){
  print('Error: ', name, ':', message);
};

/**
 * Model which abstracts the services data returned by connman. Keeps the data
 * as an array of dicts, each dict containing information about the service.
 * Also listens to changes (sent via DBus by connman) on the order of the
 * services (usually sorted by strength) and to "offline" property of each
 * service, and notify the changes to the controller.
 */
ServicesModel = EUI.Model({
  init: function() {
    this.services = [];

    manager.GetServices()
      .onComplete(function(services) {
        this.services = services;
        this.notifyListeners();
        this.addServicesChangedListener();
      }.bind(this))
      .onError(genericErrorHandler);

    manager.GetProperties()
      .onComplete(function(){
        var properties = arrayDictToDict(arguments[0]);
        this.offlineMode = properties.OfflineMode;
        this.notifyListeners();
      }.bind(this))
      .onError(genericErrorHandler);

    manager.addSignalHandler('ServicesChanged', this.servicesChanged.bind(this));
    manager.addSignalHandler('PropertyChanged', this.propertyChanged.bind(this));
  },
  length: function() {
    return this.services.length;
  },
  itemAtIndex: function(index) {
    if (!this.services[index]) return;

    var item = arrayDictToDict(this.services[index][1]);
    item.id = this.services[index][0].replace('/net/connman/service/', '');

    return item;
  },
  removeServices: function(items) {
    for (var i in items) {
      for (var j = 0; j < this.services.length; j++) {
        if (this.services[j][0] == items[i]) {
          this.services.splice(j, 1);
          break;
        }
      }
    }
  },
  servicesChanged: function(changed, removed){
    if (removed.length > 0) {
      this.removeServices(removed);
    }

    if (changed.length > 0) {
      for (var service = 0; service < this.services.length; service++) {
        for (var item = 0; item < changed.length; item++) {
          if (this.services[service][0] == changed[item][0]){
            changed[item][1] = this.services[service][1];
          }
        }
      }

      this.services = changed;
    }

    this.addServicesChangedListener();
    this.notifyListeners();
  },
  addServicesChangedListener: function() {
    for (var i in this.services) {
      if (!this.services[i][2]) {
        var obj = conn.getObject('net.connman', this.services[i][0]);
        this.services[i][2] = obj.getProxy('net.connman.Service');
        this.services[i][2].addSignalHandler('PropertyChanged',
          function(service, name, value) {
            for  (var i in service) {
              if (service[i][0] == name)
               service[i][1] = value;
            }
            this.notifyListeners();
          }.bind(this, this.services[i][1]));
      }
    }
  },
  propertyChanged: function(name, value) {
    if (name == 'OfflineMode') {
      this.offlineMode = value[0];
      this.notifyListeners();
    }
  }
});

/**
 * Controller that show a list of services. It is quite straightforward.
 * Just display the services, with icons to show type of network, signal
 * strength and security, as applicable to each service.
 */
ServicesController = EUI.ListController({
  title: 'Available networks',
  init: function() {
    this.model = new ServicesModel();
  },
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    if (!item) return;
    return {
      text: item.Name || item.id,
      icon: this.chooseIcon(item),
      end: this.chooseEnd(item)
    }
  },
  chooseEnd: function(service){
    if ((service.State != 'idle') && (service.State != 'failure'))
      return image_dir + 'connman-connected.png';

    if (service.Security && (service.Security[0] != 'none') && service.Favorite)
      return image_dir + 'connman-favorite.png';

    if (service.Security && (service.Security[0] != 'none'))
      return image_dir + 'connman-lock.png';
  },
  chooseIcon: function(service) {
    if (service.Type == 'wifi') {
      if (service.Strength >= 65)
        return image_dir + 'connman-wifi-good.png';
      if (service.Strength >= 35)
        return image_dir + 'connman-wifi-medium.png';

      return image_dir + 'connman-wifi-bad.png'
    }
    if (service.Type)
      return image_dir + 'connman-' + service.Type + '.png';
  },
  selectedItemAtIndex: function(index) {
    var service = this.model.itemAtIndex(index);
    this.pushController(new ServiceDetailsController(service));
  },
  navigationBarItems: function() {
    var state = this.model.offlineMode ? 'Offline' : 'Online';
    return {left: state, right: 'Technologies'};
  },
  selectedNavigationBarItem: function(item) {
    if (item == 'Online')
      this.goOnline(false);
    else if (item == 'Offline')
      this.goOnline(true);
    else
      this.pushController(new TechnologiesController());
  },
  goOnline: function(on) {
    manager.SetProperty('OfflineMode', dbus.Variant(!on))
      .onError(genericErrorHandler);
  }
});

/**
 * Provides service data as an array of {propery, value}. Useful to display
 * service details on a list. Also provides methods to connect/disconnect a
 * service.
 */
ServiceDetailsModel = EUI.Model({
  init: function(service) {
    this.service = service;
    this.items = [];

    var obj = conn.getObject('net.connman', this.path());
    this.service_dbus = obj.getProxy('net.connman.Service');
    this.service_dbus.addSignalHandler('PropertyChanged', function(name, value) {
      this.service[name] = arrayDictToDict([[name, value]])[name];
      this.updateItems();
    }.bind(this));

    this.updateItems();
  },
  /** Creates the array of items, based on the service dictionary */
  updateItems: function() {
    var service = this.service;
    var createItem = this.createItem;
    this.items = [
      createItem('Status', service.State, 'General'),
      createItem('Name Servers', joinArray(service.Nameservers), 'General'),
      createItem('Time Servers', joinArray(service.Timeservers), 'General'),
      createItem('Domain Names', joinArray(service.Domains), 'General'),
      createItem('Address', service.IPv4.Address, 'IPv4'),
      createItem('Netmask', service.IPv4.Netmask, 'IPv4'),
      createItem('Gateway', service.IPv4.Gateway, 'IPv4'),
      createItem('Method', service.Ethernet.Method, 'Ethernet'),
      createItem('Interface', service.Ethernet.Interface, 'Ethernet'),
      createItem('Address', service.Ethernet.Address, 'Ethernet'),
    ];

    //if wireless, good to show the Strength
    if (service.Strength)
      this.items.splice(1, 0, createItem('Strength', service.Strength, 'General'));

    this.notifyListeners();
  },
  createItem: function(title, text, group) {
    var item = {text: title + ': ' + text};
    if (group)
      item.group = group;
    return item;
  },
  itemAtIndex: function(index) {
    return this.items[index];
  },
  length: function() {
    return this.items.length;
  },
  isOnline: function() {
    return (this.service.State != 'idle') && (this.service.State != 'failure');
  },
  path: function() {
    return '/net/connman/service/' + this.service.id;
  },
  id: function() {
    return this.service.id;
  },
  disconnect: function() {
    this.service_dbus.Disconnect().onError(genericErrorHandler);
  },
  connect: function() {
    this.service_dbus.Connect().onError(genericErrorHandler);
  },
  serviceDict: function() {
    return this.service;
  },
  proxyDBus: function() {
    return this.service_dbus;
  }
});

/**
 * Display service details on a list, grouped by tipe of information.
 * Actually, the groups are defined on {@link ServiceDetailsModel}, thus this
 * controller is pretty straightforward.
 */
ServiceDetailsController = EUI.ListController({
  init: function(service, service_dbus) {
    this.title = service.Name || service.id;
    this.model = new ServiceDetailsModel(service);
    this.service_dbus = service_dbus;
  },
  itemAtIndex: function(index) {
    return this.model.itemAtIndex(index);
  },
  navigationBarItems: function() {
    var disConn = this.model.isOnline() ? 'Disconnect' : 'Connect';
    return {right: [disConn, 'Edit']}
  },
  selectedNavigationBarItem: function(item) {
    if (item == 'Disconnect')
      this.model.disconnect();
    else if (item == 'Connect')
      this.model.connect();
    else
      this.pushController(new EditServiceDetails(this.model.serviceDict(),
                                                 this.model.proxyDBus()));
  },
});

/**
 * This model handles editable service properties. It organises
 * the data to be sent and retrieved from a TableController.
 * It also takes care of properties that are arrays or dictionaries.
 */
EditableServiceDetailsModel = EUI.Model({
  init: function(service, service_dbus) {
    this.service = service;
    this.items = this.createEditableItem();
    this.service_dbus = service_dbus;
  },
  length: function(){
    return this.items.length;
  },
  itemAtIndex: function(index){
    return this.items[index];
  },
  /** The 'editable' item is a copy of the actual service data. It
   * has simple name of properties, to simplify binding to container
   */
  createEditableItem: function() {
    var service = this.service;
    var editableItem = {};
    editableItem['Nameservers'] = joinArray(service.Nameservers);
    editableItem['Timeservers'] = joinArray(service.Timeservers);
    editableItem['Domains'] = joinArray(service.Domains);
    editableItem['IPv4.Address'] = service.IPv4.Address;
    editableItem['IPv4.Netmask'] = service.IPv4.Netmask;
    editableItem['IPv4.Gateway'] = service.IPv4.Gateway;

    return [editableItem];
  },
  updateItemAtIndex: function(index, values) {
    var item = this.itemAtIndex(index);

    this.updateArrayProp('Nameservers', item, values);
    this.updateArrayProp('Timeservers', item, values);
    this.updateArrayProp('Domain', item, values);

    this.updateIPv4Props(item, values);
  },
  updateArrayProp: function(prop, before, after) {
    if (before[prop] == after[prop]) return;

    this.service_dbus
      .SetProperty(prop+'.Configuration',
                   dbus.Variant(after[prop].replace(/ /g, '').split(',')))
      .onError(genericErrorHandler);
  },
  updateIPv4Props: function(before, after, auto) {
    if (auto) {
      this.service_dbus
        .SetProperty('IPv4.Configuration',
                     dbus.Variant({Method: dbus.Variant('dhcp')}))
        .onError(genericErrorHandler);
    } else {
      var willUpdate = false;
      willUpdate = willUpdate || (before['IPv4.Address'] != after['IPv4.Address']);
      willUpdate = willUpdate || (before['IPv4.Netmask'] != after['IPv4.Netmask']);
      willUpdate = willUpdate || (before['IPv4.Gateway'] != after['IPv4.Gateway']);

      if (!willUpdate) return;

      var config = {
        Method: dbus.Variant('manual'),
        Address: dbus.Variant(after['IPv4.Address']),
        Netmask: dbus.Variant(after['IPv4.Netmask']),
        Gateway: dbus.Variant(after['IPv4.Gateway'])
      }

      this.service_dbus
        .SetProperty('IPv4.Configuration', dbus.Variant(config))
        .onError(genericErrorHandler);
    }
  }
});

/**
 * This controller provides a serie of editable entries to allow setting
 * service properties. Uses {@link EditableServiceDetailsModel} as its model
 */
EditServiceDetails = EUI.TableController({
  editable: true,
  init: function(service, service_dbus) {
    this.title = service.Name || service.id;
    this.model = new EditableServiceDetailsModel(service, service_dbus);
    this.index = 0;
    this.fields = [
      [EUI.widgets.Label({label: '<b>General</b>', expand: 'horizontal', fill: 'horizontal'})],
      [EUI.widgets.Label({label: 'Name Servers', expand: 'horizontal', fill: 'horizontal'})],
      [EUI.widgets.Entry({scrollable: true, single_line: true, field: 'Nameservers'})],
      [EUI.widgets.Label({label: 'Time Servers', expand: 'horizontal', fill: 'horizontal'})],
      [EUI.widgets.Entry({scrollable: true, single_line: true, field: 'Timeservers'})],
      [EUI.widgets.Label({label: 'Domain Names', expand: 'horizontal', fill: 'horizontal'})],
      [EUI.widgets.Entry({scrollable: true, single_line: true, field: 'Domains'})],

      [EUI.widgets.Label({label: '<b>IPv4</b>', expand: 'horizontal', fill: 'horizontal'})],
      [EUI.widgets.Label({label: 'Address', expand: 'horizontal', fill: 'horizontal'})],
      [EUI.widgets.Entry({scrollable: true, single_line: true, field: 'IPv4.Address'})],
      [EUI.widgets.Label({label: 'Netmask', expand: 'horizontal', fill: 'horizontal'})],
      [EUI.widgets.Entry({scrollable: true, single_line: true, field: 'IPv4.Netmask'})],
      [EUI.widgets.Label({label: 'Gateway', expand: 'horizontal', fill: 'horizontal'})],
      [EUI.widgets.Entry({scrollable: true, single_line: true, field: 'IPv4.Gateway'})],
   ];
  },
  navigationBarItems: {right: ['Automatic', 'Save']},
  selectedNavigationBarItem: function(item) {
    if (item == 'Automatic')
      this.model.updateIPv4Props(null, null, true);
    else
      this.model.updateItemAtIndex(this.index, this.getValues());
      this.popController();
  }
});

/**
 * Abstracts technologies data. It's much alike {@link ServicesModel},
 * but with subtle differences. For example, there is no order of technologies,
 * so, connman inform us (via DBus) that some technologie was added or
 * removed, and not a complete array with new order.
 */
TechnologiesModel = EUI.Model({
  init: function() {
    this.technologies = [];

    manager.GetTechnologies()
      .onComplete(function(technologies) {
        this.technologies = technologies;
        this.notifyListeners();
        this.addTechnologiesChangedListener();
      }.bind(this))
      .onError(genericErrorHandler);

    manager.addSignalHandler('TechnologyAdded',
                             this.technologyAdded.bind(this));
    manager.addSignalHandler('TechnologyRemoved',
                             this.technologyRemoved.bind(this));
  },
  length: function() {
    return this.technologies.length;
  },
  itemAtIndex: function(index) {
    if (!this.technologies[index]) return;

    var item = arrayDictToDict(this.technologies[index][1]);
    item.path = this.technologies[index][0];
    return item;
  },
  technologyAdded: function(path, properties) {
    this.technologies.push([path, properties]);
    this.notifyListeners();
    this.addTechnologiesChangedListener();
  },
  technologyRemoved: function(path) {
    for (var i in this.technologies) {
      if (this.technologies[i][0] == path) {
        this.technologies.splice(i, 1);
        break;
      }
    }
    this.notifyListeners();
  },
  addTechnologiesChangedListener: function() {
    for (var i in this.technologies) {
      if (!this.technologies[i][2]) {
        var obj = conn.getObject('net.connman', this.technologies[i][0]);
        this.technologies[i][2] = obj.getProxy('net.connman.Technology');
        this.technologies[i][2].addSignalHandler('PropertyChanged',
          function(service, name, value) {
            for  (var i in service) {
              if (service[i][0] == name)
               service[i][1] = value;
            }
            this.notifyListeners();
          }.bind(this, this.technologies[i][1]));
      }
    }
  },
});

/**
 * Displays a list of technologies, as served by {@link TechnologiesModel}
 */
TechnologiesController = EUI.ListController({
  init: function() {
    this.title = 'Technologies';
    this.model = new TechnologiesModel();
  },
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    if (!item) return;

    var state = 'offline';
    if (item.Connected)
      state = 'connected';
    else if (item.Powered)
      state = 'powered';

    return {
      text: item.Name,
      icon: image_dir + 'connman-tech-' + state + '.png',
      end: 'arrow_right'
    };
  },
  selectedItemAtIndex: function(index) {
    var technology = this.model.itemAtIndex(index);
    this.pushController(new TechnologyDetailsController(technology));
  }
});

/**
 * Handles data about a technology. Also provides methods to interact
 * with it, like scan and powerOn[Off]
 */
TechnologyDetailsModel = EUI.Model({
  init: function(technology) {
    this.technology = technology;
    this.items = [];

    var obj = conn.getObject('net.connman', technology.path);
    this.dbus_proxy = obj.getProxy('net.connman.Technology');
    this.dbus_proxy.addSignalHandler('PropertyChanged',
                                     this.propertyChanged.bind(this));

    this.recreateItems();
  },
  recreateItems: function() {
    this.items[0] = this.technology.Connected ? 'Connected' : 'Not connected';
    this.items[1] = this.technology.Powered ? 'Powered on' : 'Powered off ';
    this.notifyListeners();
  },
  powered: function() {
    return this.technology.Powered;
  },
  propertyChanged: function(name, value) {
    this.technology[name] = value[0];
    this.recreateItems();
  },
  length: function() {
    return this.items.length;
  },
  itemAtIndex: function(index) {
    return this.items[index];
  },
  powerOn: function(on) {
    this.dbus_proxy.SetProperty('Powered', dbus.Variant(on))
      .onError(genericErrorHandler);
  },
  scan: function() {
    this.dbus_proxy.Scan().onError(genericErrorHandler);
  }
});

/**
 * Display technology details based on {@link TechnologyDetailsModel}.
 */
TechnologyDetailsController = EUI.ListController({
  init: function(technology) {
    this.title = technology.Name;
    this.model = new TechnologyDetailsModel(technology);
  },
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {
      text: item
    }
  },
  navigationBarItems: function() {
    var powered = this.model.powered() ? 'Power off' : 'Power on';
    return {right: [powered, 'Scan']};
  },
  selectedNavigationBarItem: function(item) {
    if (item == 'Scan')
      this.model.scan();
    else
      this.model.powerOn(item == 'Power on');
  }
});

EUI.app(new ServicesController());
