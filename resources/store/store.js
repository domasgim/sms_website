import Vue from "vue"
import Vuex from "vuex"
import axios from "axios"
import router from '../router'

Vue.use(Vuex);

export const store = new Vuex.Store({
    state: {
        receivedMessages: [],
        sentMessages: [],
        pageCountReceived: 0,
        pageCountSent: 0,
        currentPage: 1,
        itemsPerPage: 5,
        filterNumber: '',
        filterContent: '',
        filterDate: '',
    },
    getters: {
        getReceivedMessages (state) {
            let filteredMessages = state.receivedMessages
            if (state.filterNumber !== '') {
                filteredMessages = filteredMessages.filter(item => item.sender.includes(state.filterNumber))
            }
            if (state.filterContent !== '') {
                filteredMessages = filteredMessages.filter(item => item.content.toLowerCase().includes(state.filterContent.toLowerCase()))
            }
            if (state.filterDate !== '') {
                filteredMessages = filteredMessages.filter(item => item.created_at.includes(state.filterDate))
            }
            state.pageCountReceived = Math.ceil(filteredMessages.length / state.itemsPerPage)
            const offset = (state.currentPage - 1) * state.itemsPerPage
            return filteredMessages.slice(offset, state.itemsPerPage + offset)
        },
        getSentMessages (state) {
            let filteredMessages = state.sentMessages
            if (state.filterNumber !== '') {
                filteredMessages = filteredMessages.filter(item => item.recipient.includes(state.filterNumber))
            }
            if (state.filterContent !== '') {
                filteredMessages = filteredMessages.filter(item => item.content.toLowerCase().includes(state.filterContent.toLowerCase()))
            }
            if (state.filterDate !== '') {
                filteredMessages = filteredMessages.filter(item => item.created_at.includes(state.filterDate))
            }
            state.pageCountSent = Math.ceil(filteredMessages.length / state.itemsPerPage)
            const offset = (state.currentPage - 1) * state.itemsPerPage
            return filteredMessages.slice(offset, state.itemsPerPage + offset)
        },
        getUnreadMessages (state) {
            return state.receivedMessages.filter((item) => {
                return item.read == false
            })
        }
    },
    mutations: {
        initReceivedMessages (state) {
            axios.get('/receivedMessages')
            .then((response) => {
                state.receivedMessages = response.data.received_messages
            })
            .catch((err) => {
                alert('there was an error loading received messages!')
            })
        },
        initSentMessages (state) {
            axios.get('/sentMessages')
            .then((response) => {
                state.sentMessages = response.data.sent_messages
                state.pageCountSent = Math.ceil(response.data.sent_messages.length / state.itemsPerPage)
            })
            .catch((err) => {
                alert('there was an error loading sent messages!')
            })
        },
        sendMessage (state, payload) {
            axios.post('/sentMessages', payload)
            .then((response) => {
                state.sentMessages = [response.data.response, ...state.sentMessages]
                state.pageCountSent = Math.ceil(state.sentMessages.length / state.itemsPerPage)
                alert(response.data.message)
                router.push('sent')
            })
            .catch((err) => {
                alert(err.response.data.errors.recipient[0])
            })
        },
        updateReadStatus (state, payload) {
            axios.put(`/receivedMessages/${payload.id}`, {
                read: payload.read
            })
            .then(response => {
                state.receivedMessages.forEach((item) => {
                    if (item.id == payload.id) {
                        item.read = payload.read
                    }
                })
            })
            .catch ((err) => {
                alert('there was an error changing message read status!')
            })
        },
        updateCurrentPage (state, nr) {
            state.currentPage = nr;
        },
        setFilterNumber (state, payload) {
            state.filterNumber = payload
            state.currentPage = 1
        },
        setFilterContent (state, payload) {
            state.filterContent = payload
            state.currentPage = 1
        },
        setFilterDate (state, payload) {
            state.filterDate = payload
            state.currentPage = 1
        },
    },
    actions: {
        initStore (context) {
            context.commit('initReceivedMessages')
            context.commit('initSentMessages')
        }
    }
});
