FROM node:20-alpine

WORKDIR /app

ENV NODE_ENV=production
ENV HOST=0.0.0.0
ENV PORT=8080

COPY package.json server.js ./

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
  CMD node -e "const http=require('node:http');const port=process.env.PORT||8080;const req=http.get({host:'127.0.0.1',port,path:'/health',timeout:3000},res=>process.exit(res.statusCode===200?0:1));req.on('error',()=>process.exit(1));req.on('timeout',()=>req.destroy());"

USER node

CMD ["node", "server.js"]
