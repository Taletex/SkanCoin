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
  
  # OBSERVING VARIABLES AND EVENTS #
  v <- reactiveValues(query = 0, title = "", data = "")
  
  observeEvent(input$query1, {
    v$query <- 1
    v$title <- "Statistiche temporali della blockchain (numero di blocchi, numero di transazioni e numero di coin)"
    v$data <- getStats("blockchainstats")
  })
  
  observeEvent(input$query2, {
    v$query <- 2
    v$title <- "Tempo di mining di ogni blocco"
    v$data <- getStats("transminingtime")
  })
  
  observeEvent(input$query3, {
    v$query <- 3
    v$title <- "Tempo di attesa per la conferma di ogni transazione"
    v$data <- getStats("transactionwaitingtime")
  })
  
  
  # OUTPUTS #
  output$title <- renderText({ 
    v$title
  })
  
  output$statgraph <- renderPlot({
    
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
      x <- factor(unlist(v$data["transactionId"], use.names=FALSE), levels = unlist(v$data["transactionId"], use.names=FALSE))
      y <- unlist(v$data["millisWaitTime"], use.names=FALSE)
      ggplot(v$data, aes(x, y)) + 
        geom_bar(stat="identity", width=.3, fill="darkblue") + 
        geom_text(aes(x, y, label=y, hjust=0.5, vjust=-1)) +
        labs(x = "Transazioni", y = "Tempo di attesa") +
        theme_bw() 
    }
  })
  
  # HTTP REQUEST FUNCTION #
  getStats = function(type) {
    obj <- fromJSON(paste("http://localhost:3001/webresources/stats/", type, sep=""))  
    if(typeof(obj$data) == "list") {
      obj <- obj$data
    } else {
      v$title <- obj$message
      print(v$title)
      obj <- getEmptyDataFrame(v$query)
    }
  }
  
  getEmptyDataFrame = function(type) {
    if(type == 1) data.frame("time" = c(0), "blocks" = c(0), "transactions" = c(0), "coins" = c(0))
    else if(type == 2) data.frame("block" = c(0), "miningtime" = c(0))
    else if(type == 3) data.frame("transactionId" = c(0), "millisWaitTime" = c(0))
  }
}

# Run the app ----
shinyApp(ui = ui, server = server)
