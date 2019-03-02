library(shiny)    
library(ggplot2)
library(jsonlite)

# Define UI ----
ui <- fluidPage(
  titlePanel(
    "Diagnostic R Client"
  ),

  sidebarLayout(
    sidebarPanel(
      h2("Queries"),
      helpText("Seleziona la query che vuoi effettuare!"),
      
      div(
        div("Grafica statistiche temporali della blockchain (numero blocchi, transazioni e coin)"),
        actionButton("query1", "Invia", style="margin-left: auto"),
        style="margin-bottom:10px; display: flex; align-items: center"
      ), 
      div(
        div("Grafica tempo di mining per ogni blocco"),
        actionButton("query2", "Invia", style="margin-left: auto"),
        style="margin-bottom:10px; display: flex; align-items: center"
      ),
      div(
        div("Grafica tempo di attesa per la conferma di ogni transazione"),
        actionButton("query3", "Invia", style="margin-left: auto"),
        style="margin-bottom:10px; display: flex; align-items: center"
      ),
      
      style="align-items: center;"
    ),
    
    mainPanel(
      h2("Results"),
      textOutput("title"),
      br(),
      plotOutput("statgraph")
    )
  )
)

# Define server logic ----
server <- function(input, output) {
  
  # INIT DATA FILE #
  #stat <- data.frame("time"=c(0), "blocks"=c(1), "transactions"=c(0), "coins"=c(0))
  #save(stat,file="/home/alessandro/Projects/SkanCoin/DiagnosticClient/app/data/data.Rda")
  #load(file="/home/alessandro/Projects/SkanCoin/DiagnosticClient/app/data/data.Rda")     now "stat" variable contains the saved dataframe
  
  
  # OBSERVING VARIABLES AND EVENTS #
  v <- reactiveValues(query = 0, title = "", data = "")
  
  observeEvent(input$query1, {
    v$query <- 1
    v$title <- "Statistiche temporali della blockchain (numero di blocchi, numero di transazioni e numero di coin)"
    v$data <- getStats()
  })
  
  observeEvent(input$query2, {
    v$query <- 2
    v$title <- "Tempo di mining di ogni blocco"
    v$data <- getBlocksMiningTime()
  })
  
  observeEvent(input$query3, {
    v$query <- 3
    v$title <- "Tempo di attesa per la conferma di ogni transazione"
    v$data <- getWaitingTransactionMiningTime()
  })
  
  
  # OUTPUTS #
  output$title <- renderText({ 
    v$title
  })
  
  output$statgraph <- renderPlot({
    # Commenti da rimuovere
    #legend(1, 50, legend = c("Blocchi", "Transazioni", "Coins"), col = c("blue", "red", "black"), pch=c("o", "*", "+"), lty = c(1, 2, 3), ncol=2)
    #x <- ts(v$data["time"])
    #y1 <- ts(v$data["blocks"])
    #y2 <- ts(v$data["transactions"])
    #y3 <- ts(v$data["coins"])
    #plot(x, y1, main="Blocchi", xlab = "Time", type="o", col="blue", pch="o", lty=1)
    #points(x, y2, col="red", pch="*")
    #lines(x, y2, col="red", lty=2)
    #points(x, y3, col="dark red", pch="+")
    #lines(x, y3, col="dark red", lty=3)
    
    if(v$query == 1) {
      # Blockchain stats
      x <- unlist(v$data["time"], use.names=FALSE)
      y1 <- unlist(v$data["blocks"], use.names=FALSE)
      y2 <- unlist(v$data["transactions"], use.names=FALSE)
      y3 <- unlist(v$data["coins"], use.names=FALSE)
      df1 <- data.frame(x, y1)
      df2 <- data.frame(x, y2)
      df3 <- data.frame(x, y3)
      ggplot() + 
        geom_line(data=df1, aes(x, y1, color="Blocks"), size=1, linetype="dashed") + geom_point(data=df1, aes(x, y1, color="Blocks"), size=2, shape=15) + geom_text(aes(x, y1, label=y1, color="Blocks"), hjust=1, vjust=1.8) +
        geom_line(data=df2, aes(x, y2, color="Transactions"), size=1, linetype="dashed") + geom_point(data=df2, aes(x, y2, color="Transactions"), size=2, shape=16) + geom_text(aes(x, y2, label=y2, color="Transactions"), hjust=1, vjust=-1) +
        geom_line(data=df3, aes(x, y3, color="Coins"), size=1, linetype="dashed") + geom_point(data=df3, aes(x, y3, color="Coins"), size=2, shape=17) + geom_text(aes(x, y3, label=y3, color="Coins"), hjust=1, vjust=-1) +
        labs(x = "Tempo", y = "Valori", color="Legenda") +
        theme_bw() +
        theme(legend.position="right", legend.title=element_text(size=15), legend.key.size=unit(0.5, "cm"), legend.key.width=unit(2, "cm")) +
        guides(col = guide_legend(nrow=3,byrow=TRUE), linetype=FALSE)
    } else if(v$query == 2) {
      # Block mining time
      x <- factor(unlist(v$data["block"], use.names=FALSE), levels = unlist(v$data["block"], use.names=FALSE))
      y <- unlist(v$data["miningtime"], use.names=FALSE)
      ggplot(v$data, aes(x, y)) + 
        geom_bar(stat="identity", width=.3, fill="darkred") + 
        geom_text(aes(x, y, label=y, hjust=0.5, vjust=-1)) +
        labs(x = "Blocchi", y = "Tempo di mining") +
        theme_bw() 
    } else if(v$query == 3) {
      # Transactions waiting time
      x <- factor(unlist(v$data["transaction"], use.names=FALSE), levels = unlist(v$data["transaction"], use.names=FALSE))
      y <- unlist(v$data["waitingtime"], use.names=FALSE)
      ggplot(v$data, aes(x, y)) + 
        geom_bar(stat="identity", width=.3, fill="darkblue") + 
        geom_text(aes(x, y, label=y, hjust=0.5, vjust=-1)) +
        labs(x = "Transazioni", y = "Tempo di attesa") +
        theme_bw() 
    }
  })
  
  
  # HTTP REQUEST FUNCTIONS #
  getStats = function() {
    #obj <- fromJSON("http://localhost:3001/webresources/stats/blockchainstats")                   # bisogna controllare che obj sia una lista con i 4 elementi delle statistiche!
    load(file="/home/alessandro/Projects/SkanCoin/DiagnosticClient/app/data/data.Rda")  
    #rbind(stat, obj)
    #save(stat,file="/home/alessandro/Projects/SkanCoin/DiagnosticClient/app/data/data.Rda")
    # TOD: cambiare tutto, ora ritorna un array di oggetti!
    stat
  }
  
  getBlocksMiningTime = function() {
    #obj <- fromJSON("http://localhost:3001/webresources/stats/blocksminingtime")                   # bisogna controllare cosa restituisce
    stat <- data.frame("block"=c(1, 2, 3, 4, 5, 6, 7), "miningtime"=c(12, 15, 20, 11, 30, 12, 10))
  }
  
  getWaitingTransactionMiningTime = function() {
    #obj <- fromJSON("http://localhost:3001/webresources/transminingtime")                    # bisogna controllare cosa restituisce
    stat <- data.frame("transaction"=c(1, 2, 3, 4, 5, 6, 7), "waitingtime"=c(1, 5, 5, 50, 12, 134, 73))
  }
}

# Run the app ----
shinyApp(ui = ui, server = server)
