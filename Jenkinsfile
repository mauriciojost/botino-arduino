// https://jenkins.io/doc/book/pipeline/jenkinsfile/
// Scripted pipeline (not declarative)
pipeline {
  agent {
    docker { 
      image 'mauriciojost/arduino-ci:platformio-3.5.3-0.1.0' 
    }
  }
  stages {
    stage('Build') {
      steps {
        script {
          sshagent(['bitbucket_key']) {
            sh 'export GIT_COMMITTER_NAME=mjost && export GIT_COMMITTER_EMAIL=mauriciojost@gmail.com && set && ./pull_dependencies'
            sh 'platformio run'
          }
        }
      }
    }
    stage('Test') {
      steps {
        sh './launch_tests'
      }
    }
  }
  post {  
    failure {  
      emailext body: "<b>[JENKINS] Failure</b>Project: ${env.JOB_NAME} <br>Build Number: ${env.BUILD_NUMBER} <br> Build URL: ${env.BUILD_URL}", from: '', mimeType: 'text/html', replyTo: '', subject: "ERROR CI: ${env.JOB_NAME}", to: "mauriciojostx@gmail.com", attachLog: true, compressLog: false;
    }  
    success {  
      emailext body: "<b>[JENKINS] Success</b>Project: ${env.JOB_NAME} <br>Build Number: ${env.BUILD_NUMBER} <br> Build URL: ${env.BUILD_URL}", from: '', mimeType: 'text/html', replyTo: '', subject: "SUCCESS CI: ${env.JOB_NAME}", to: "mauriciojostx@gmail.com", attachLog: false, compressLog: false;
    }  
  }
}
