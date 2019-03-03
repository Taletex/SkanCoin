var app = angular.module('myApp', []);
app.controller('mainCtrl', function($scope, $http, $httpParamSerializerJQLike) {

  $scope.baseUrl = "http://localhost:3001/webresources/";
  $scope.bLoading = false;
  $scope.inputs = {};
  $scope.queryOutput = null;
  $scope.pubblicKey = "";

  /* === COLLECTION REST === */

  // Ritorna la chiave pubblica del wallet
  $scope.getPubblicKey = function() {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "pubblickey").then(function(resp) {
      $scope.pubblicKey = resp.data.pubblickey;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

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
    $http.get($scope.baseUrl + "blocks/" + blockHash).then(function(resp) {
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
    $http.get($scope.baseUrl + "transactions/" + transactionId).then(function(resp) {
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

  // Ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)
  $scope.getWalletUnspentTransactionOutputs = function(address) {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "unspentTransactionOutputs/" + address).then(function(resp) {
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

  // Richiama una REST per effettuare il mining di un nuovo blocco utilizzando le transazioni passate come argomento
  $scope.mineBlockWithTransactions = function(transactions) {
    if(transactions != null) {
      $scope.bLoading = true;
      $http.post($scope.baseUrl + "blocks/transactions", {data: transactions}, {headers:{'Content-Type': 'application/json'}}).then(function(resp) {
        document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
        $scope.queryOutput = resp.data;
        console.log(resp);
      }).finally (function(){
        $scope.bLoading = false;
      });
    }
  };

  // Richiama una REST per effettuare il mining di un nuovo blocco utilizzando le transazioni del transaction pool
  $scope.mineBlockWithTransactionPool = function() {
    $scope.bLoading = true;
    $http.post($scope.baseUrl + "blocks/pool").then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  // Effettua il mining di un nuovo blocco utilizzando una nuova transazione creata a partire dall'amount e address passati (più la coinbase transaction)
  $scope.mineBlockWithTransaction = function(address, amount) {
    if(address != null && amount != null) {
      $scope.bLoading = true;
      $http.post($scope.baseUrl + "blocks/transaction", {address: address, amount: amount}, {headers:{'Content-Type': 'application/json'}}).then(function(resp) {
        document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
        $scope.queryOutput = resp.data;
        console.log(resp);
      }).finally (function(){
        $scope.bLoading = false;
      });
    }
  };

  // Crea una nuova transazione e la inserisce nel transaction pool
  $scope.sendTransaction = function(address, amount) {
    if(address != null && amount != null) {
      $scope.bLoading = true;
      $http.post($scope.baseUrl + "transactions", {address: address, amount: amount}, {headers:{'Content-Type': 'application/json'}}).then(function(resp) {
        document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
        $scope.queryOutput = resp.data;
        console.log(resp);
      }).finally (function(){
        $scope.bLoading = false;
      });
    }
  };

  // Ritorna il balance del wallet
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
  $scope.addPeer = function(Ipaddr, peerPort) {
    if(Ipaddr != null) {
      $scope.bLoading = true;
      $http.post($scope.baseUrl + "peers", {peer: "ws://" + Ipaddr + ":" + (peerPort != null ? peerPort : "8080")}, {headers:{'Content-Type': 'application/json'}}).then(function(resp) {
        document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
        $scope.queryOutput = resp.data;
        console.log(resp);
      }).finally (function(){
        $scope.bLoading = false;
      });
    }
  };

  // Ritorna il numero di peer cui è connesso il nodo
  $scope.getPeers = function() {
    $scope.bLoading = true;
    $http.get($scope.baseUrl + "peers").then(function(resp) {
      document.getElementById("output").innerHTML = syntaxHighlight(JSON.stringify(resp.data, undefined, 4));
      $scope.queryOutput = resp.data;
      console.log(resp);
    }).finally (function(){
      $scope.bLoading = false;
    });
  };

  /* ALTRO */
  // Per la formattazione dell'output
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

  $scope.getPubblicKey();
});
