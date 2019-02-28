var app = angular.module('myApp', []);
app.controller('mainCtrl', function($scope, $http, $httpParamSerializerJQLike) {

  $scope.baseUrl = "http://localhost:3001/webresources/";
  $scope.bLoading = false;
  $scope.inputs = {};
  $scope.queryOutput = null;


  /* === COLLECTION REST === */

  // Ritorna una stringa contenente la blockchain
  $scope.getBlockchain = function() {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "blocks").then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  // Ritorna un blocco della blockchain dato il suo hash
  $scope.getBlockFromHash = function(blockHash) {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "block/" + blockHash).then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  // Ritorna una transazione della blockchain dato il suo id
  $scope.getTransactionFromId = function(transactionId) {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "transaction/" + transactionId).then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  // Ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)
  $scope.getWalletUnspentTransactionOutputs = function(address) {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "address/" + address).then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  // Ritorna la lista degli output non spesi dell'intera blockchain
  $scope.getBlockchainUnspentTransactionOutputs = function() {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "unspentTransactionOutputs").then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  // Ritorna la lista degli output non spesi dal wallet attuale
  $scope.getMyUnspentTransactionOutputs = function() {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "myUnspentTransactionOutputs").then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  // Effettua il mine di un raw block (TODO: Controllare bene cosa bisogna passargli)
  $scope.mineRawBlock = function(transactions) {
    if(transactions != null) {
      $scope.bLoading = true;
      $http.post($scope.baseUrl + "mineRawBlock/write", $httpParamSerializerJQLike({collection_name: collectionName, directory: directory, cycle: cycle, mean_add: meanAdd, mean_download: meanDownload, stddev_add: stdDevAdd, stddev_download: stdDevDownload, state: state, timestamp: moment().format("YYYY/MM/DD - HH:mm:ss")}), {headers:{'Content-Type': 'application/x-www-form-urlencoded'}}).then(function(resp) {
        document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
        $scope.queryOutput = resp.data;
        console.log(resp);
      }).finally (function(){
        $scope.bLoading = false;
      });
    }
  };

  // Effettua il mine di un blocco normale (senza transazioni)
  $scope.mineBlock = function() {
    $scope.bLoading = true;
    $http.post($scope.baseUrl + "mineBlock").then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  // Ritorna il bilancio del wallet attuale
  $scope.getMyBalance = function() {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "balance").then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };


  // Effettua il mine di una transazione (TODO: Controllare bene cosa bisogna passargli)
  $scope.mineTransaction = function(address, amount) {
    if(address != null && amount != null) {
      $scope.bLoading = true;
      $http.post($scope.baseUrl + "mineTransaction", $httpParamSerializerJQLike({address: address, amount: amount}), {headers:{'Content-Type': 'application/x-www-form-urlencoded'}}).then(function(resp) {
        document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
        $scope.queryOutput = resp.data;
        console.log(resp);
      }).finally (function(){
        $scope.bLoading = false;
      });
    }
  };

  // Invia una transazione (TODO: Controllare bene cosa bisogna passargli)
  $scope.sendTransaction = function(address, amount) {
    if(address != null && amount != null) {
      $scope.bLoading = true;
      $http.post($scope.baseUrl + "sendTransaction", $httpParamSerializerJQLike({address: address, amount: amount}), {headers:{'Content-Type': 'application/x-www-form-urlencoded'}}).then(function(resp) {
        document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
        $scope.queryOutput = resp.data;
        console.log(resp);
      }).finally (function(){
        $scope.bLoading = false;
      });
    }
  };

  // Ritorna la transaction pool del nodo
  $scope.getTransactionPool = function() {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "transactionPool").then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  // Aggiunge un peer alla lista di peer
  $scope.addPeer = function(peer) {
    if(peer != null) {
      $scope.bLoading = true;
      $http.post($scope.baseUrl + "peer", $httpParamSerializerJQLike({peer: peer}), {headers:{'Content-Type': 'application/x-www-form-urlencoded'}}).then(function(resp) {
        document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
        $scope.queryOutput = resp.data;
        console.log(resp);
      }).finally (function(){
        $scope.bLoading = false;
      });
    }
  };

  // Ritorna i peer del nodo
  $scope.getPeer = function() {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "peers").then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  /* Others */
  function syntaxHighlight(json) {
      json = json.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
      return json.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g, function (match) {
          var cls = 'number';
          if (/^"/.test(match)) {
              if (/:$/.test(match)) {
                  cls = 'key';
              } else {
                  cls = 'string';
              }
          } else if (/true|false/.test(match)) {
              cls = 'boolean';
          } else if (/null/.test(match)) {
              cls = 'null';
          }
          return '<span class="' + cls + '">' + match + '</span>';
      });
  }

});
