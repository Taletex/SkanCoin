library(shiny)
require(graphics) 

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
      plotOutput("blocks"),
      plotOutput("transactions"),
      plotOutput("coins")
    )
  )
)

# Define server logic ----
server <- function(input, output) {
  
  # INIT DATA FILE #
  #stat <- data.frame("time"=c(0), "blocks"=c(1), "transactions"=c(0), "coins"=c(0))
  #save(stat,file="/home/alessandro/Projects/SkanCoin/DiagnosticClient/app/data/data.Rda")
  #load(file="/home/alessandro/Projects/SkanCoin/DiagnosticClient/app/data/data.Rda")     now "stat" variable contains the saved dataframe
  
  
  # OBSERVING VARIABLES #
  v <- reactiveValues(data = "")
  
  observeEvent(input$query1, {
    load(file="/home/alessandro/Projects/SkanCoin/DiagnosticClient/app/data/data.Rda")
    
    v$query <- 1
    v$title <- "Statistiche temporali della blockchain"
    v$data <- stat
  })
  
  observeEvent(input$query2, {
    v$query <- 2
    v$title <- "Tempo di mining di ogni blocco"
    v$data <- ""
  })
  
  observeEvent(input$query3, {
    v$query <- 3
    v$title <- "Tempo di attesa per la conferma di ogni transazione"
    v$data <- ""
  })
  
  
  # OUTPUTS #
  output$title <- renderText({ 
    v$title
  })
  
  output$blocks <- renderPlot({
    x <- ts(v$data["time"])
    y <- ts(v$data["blocks"])
    plot.ts(x, y, main = v$title, xlab = "Time", ylab = "Value")
    points(x,y)
    #legend("top", legend = "Lunghezza della Blockchain", col = "black", lty = 1)
  })
  
  output$transactions <- renderPlot({
    x <- ts(v$data["time"])
    y <- ts(v$data["transactions"])
    plot.ts(x, y, main = v$title, xlab = "Time", ylab = "Value")
    points(x,y)
    #legend("top", legend = "Lunghezza della Blockchain", col = "black", lty = 1)
  })
  
  output$coins <- renderPlot({
    x <- ts(v$data["time"])
    y <- ts(v$data["coins"])
    plot.ts(x, y, main = v$title, xlab = "Time", ylab = "Value")
    points(x,y)
    #legend("top", legend = "Lunghezza della Blockchain", col = "black", lty = 1)
  })
  
}

# Run the app ----
shinyApp(ui = ui, server = server)
