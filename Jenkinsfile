// https://jenkins.io/doc/book/pipeline/jenkinsfile/
// Scripted pipeline (not declarative)
pipeline {
  agent {
    docker { 
      image 'mauriciojost/arduino-ci:latest' 
    }
  }
  stages {
    stage('Build') {
      //when { expression { env.BRANCH_NAME != 'master' } }
      steps {
        script {
          sshagent(['bitbucket_key']) {
            echo "My branch is: ${env.BRANCH_NAME}"
            sh 'whoami'
            sh 'pwd'
            sh 'echo $HOME'
            sh 'ls -lah $HOME'
            sh 'ls -lah $HOME/.ssh'
            sh 'export GIT_COMMITTER_NAME=mjost && export GIT_COMMITTER_EMAIL=mauriciojost@gmail.com && set && ./pull_dependencies'
            sh 'platformio run'
          }
        }
      }
    }
    stage('Test') {
      steps {
        echo "My branch is: ${env.BRANCH_NAME}"
        sh './launch_testsX'
      }
    }
  }
  post {  
    failure {  
      emailext bcc: '', body: "<b>[JENKINS] Failure</b>Project: ${env.JOB_NAME} <br>Build Number: ${env.BUILD_NUMBER} <br> Build URL: ${env.BUILD_URL}", cc: '', charset: 'UTF-8', from: '', mimeType: 'text/html', replyTo: '', subject: "ERROR CI: ${env.JOB_NAME}", to: "mauriciojostx@gmail.com", attachLog: true, compressLog: true;
    }  
  }
}
