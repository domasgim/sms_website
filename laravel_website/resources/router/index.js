import Vue from 'vue'
import Router from 'vue-router'
import SendMessage from "../views/SendMessage.vue";
import Messages from "../views/Messages.vue";

Vue.use(Router)

export default new Router({
    routes: [
        {
            path: "/",
            name: "ReceivedMessages",
            component: Messages
        },
        {
            path: "/sent",
            name: "SentMessages",
            component: Messages,
            props: {
                type: 'sent'
            }
        },
        {
            path: "/create",
            name: "SendMessage",
            component: SendMessage
        }
    ]
});
